#include <iostream>
#include <windows.h>
#include <tchar.h>
#include "SnapModule.h"
#include "config.h"
#include "main.h"

using namespace std;

static const UINT FILE_CODEPAGE = CP_ACP;

void TrimSpaces(TCHAR*& str)
{
    // 앞 공백 제거
    while (*str && _istspace(*str)) str++;
    // 뒤 공백 제거
    TCHAR* end = str + _tcslen(str) - 1;
    while (end > str && _istspace(*end)) *end-- = _T('\0');
}

DWORD ConfigToken(PTDATA pda)
{
    // --- 1) 파일 열기 및 바이트 읽기 ---
    HANDLE hFile = CreateFileA(CONFIG_FILE,
        GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return -1;

    DWORD dwBytes = GetFileSize(hFile, NULL);
    if (dwBytes == INVALID_FILE_SIZE) { CloseHandle(hFile); return -1; }

    // 바이트 버퍼에 읽기
    char* byteBuf = new char[dwBytes + 1];
    DWORD rd = 0;
    if (!byteBuf ||
        !ReadFile(hFile, byteBuf, dwBytes, &rd, NULL) ||
        rd != dwBytes)
    {
        delete[] byteBuf;
        CloseHandle(hFile);
        return -1;
    }
    byteBuf[rd] = '\0';
    CloseHandle(hFile);

    // --- 2) 멀티바이트→유니코드 변환 ---
    int  wlen = MultiByteToWideChar(CP_ACP, 0,
        byteBuf, -1,
        NULL, 0);
    if (wlen <= 0) { delete[] byteBuf; return -1; }

    TCHAR* wbuf = new TCHAR[wlen];
    MultiByteToWideChar(CP_ACP, 0,
        byteBuf, -1,
        wbuf, wlen);
    delete[] byteBuf;

    // --- 3) 초기화 ---
    pda->index = 0;
    pda->dllIndex = 0;

    // --- 4) 한 줄씩 파싱 (줄 구분자: CR/LF) ---
    TCHAR* lineCtx = NULL;
    TCHAR* line = _tcstok_s(wbuf, _T("\r\n"), &lineCtx);

    while (line)
    {
        // 키=값 형태인지 확인
        TCHAR* eq = _tcschr(line, _T('='));
        if (eq)
        {
            *eq = _T('\0');
            TCHAR* key = line;
            TCHAR* val = eq + 1;
            TrimSpaces(key);
            TrimSpaces(val);

            // 쉼표 단위 토큰화
            TCHAR* tokCtx = NULL;
            TCHAR* token = _tcstok_s(val, _T(",\t "), &tokCtx);

            if (_tcsicmp(key, _T("target")) == 0)
            {
                // target = ... → pda->Token 에 저장
                while (token && pda->index < TOKEN_SIZE)
                {
                    TrimSpaces(token);
                    pda->Token[pda->index++] = _tcsdup(token);
                    token = _tcstok_s(NULL, _T(",\t "), &tokCtx);
                }
            }
            else if (_tcsicmp(key, _T("module")) == 0)
            {
                // module = ... → pda->DllToken 에 저장
                while (token && pda->dllIndex < TOKEN_SIZE)
                {
                    TrimSpaces(token);
                    pda->DllToken[pda->dllIndex++] = _tcsdup(token);
                    token = _tcstok_s(NULL, _T(",\t "), &tokCtx);
                }
            }
        }

        line = _tcstok_s(NULL, _T("\r\n"), &lineCtx);
    }

    delete[] wbuf;
    return 0;
}