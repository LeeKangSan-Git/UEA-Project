#include <iostream>
#include <windows.h>
#include <tchar.h>
#include "SnapModule.h"
#include "config.h"
#include "main.h"

using namespace std;

static const UINT FILE_CODEPAGE = CP_ACP;

DWORD ConfigToken(PTDATA pda)
{
    // 1) 파일 열기
    HANDLE hFile = CreateFileA(
        CONFIG_FILE,
        GENERIC_READ, FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return -1;

    // 2) 파일 크기(바이트) 얻기
    DWORD dwFileSize = GetFileSize(hFile, NULL);
    if (dwFileSize == INVALID_FILE_SIZE)
    {
        CloseHandle(hFile);
        return -1;
    }

    // 3) 바이트 단위로 읽기
    char* byteBuf = new char[dwFileSize + 1];
    DWORD  bytesRead = 0;
    if (!byteBuf ||
        !ReadFile(hFile, byteBuf, dwFileSize, &bytesRead, NULL) ||
        bytesRead != dwFileSize)
    {
        delete[] byteBuf;
        CloseHandle(hFile);
        return -1;
    }
    byteBuf[bytesRead] = '\0';
    CloseHandle(hFile);

    // 4) 멀티바이트 → 유니코드(WCHAR) 변환
    int    wchCount = MultiByteToWideChar(
        FILE_CODEPAGE,    // CP_ACP or CP_UTF8
        0,
        byteBuf,
        -1,
        NULL, 0);
    if (wchCount <= 0)
    {
        delete[] byteBuf;
        return -1;
    }

    wchar_t* wbuf = new wchar_t[wchCount];
    MultiByteToWideChar(
        FILE_CODEPAGE,
        0,
        byteBuf,
        -1,
        wbuf,
        wchCount);
    delete[] byteBuf;

    // 5) '=' 이후부터 토큰화
    wchar_t* p = wcschr(wbuf, L'=');
    if (!p)
    {
        delete[] wbuf;
        return -1;
    }
    p++;                              // 등호 건너뛰기
    while (*p == L' ') p++;           // 공백 건너뛰기

    // 6) 안전한 토큰화를 위한 컨텍스트 변수
    wchar_t* context = NULL;
    wchar_t* token = wcstok_s(p, L",\r\n", &context);

    // 7) pda에 문자열 복제 저장
    int idx = 0;
    for (; token && idx < TOKEN_SIZE; token = wcstok_s(NULL, L",\r\n", &context))
    {
        size_t len = wcslen(token);
        pda->Token[idx] = new TCHAR[len + 1];
        // 유니코드 빌드라면 wcscpy_s, 멀티바이트 빌드라면 strcpy_s로 매핑됩니다
        _tcscpy_s(pda->Token[idx], len + 1, token);
        idx++;
    }
    pda->index = idx;

    // 8) 임시 버퍼 해제
    delete[] wbuf;
    return 0;
}