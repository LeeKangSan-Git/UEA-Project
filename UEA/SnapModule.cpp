#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include "main.h"
#include "SnapModule.h"

using namespace std;

DWORD GetProcId(LPCTSTR Name)
{
	DWORD pId = 0;
	HANDLE hProcSnap;
	PROCESSENTRY32 ProcEntry;

	hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (hProcSnap == INVALID_HANDLE_VALUE)
		return 0;

	ProcEntry.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hProcSnap, &ProcEntry);
	do
	{
		if (!_tcsncmp(ProcEntry.szExeFile, Name, sizeof(Name)))
		{
			pId = ProcEntry.th32ProcessID;
			break;
		}
	} while (Process32Next(hProcSnap, &ProcEntry));
	//루프를 다 돌았는데 찾는 프로세스가 없다면 Process32Next는 FALSE를 리턴 그리고 pid는 0을 반환

	CloseHandle(hProcSnap);
	return pId;
}

DWORD CheckModule(PTDATA pda, DWORD pId)
{
	HANDLE hModSnap;
	BOOL ch = FALSE;
	MODULEENTRY32 ModEntry;
	int i = 0;

	hModSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pId);
	if (hModSnap == INVALID_HANDLE_VALUE)
		return MD_ERROR;

	ModEntry.dwSize = sizeof(MODULEENTRY32);
	Module32First(hModSnap, &ModEntry);
	do
	{
		ch = FALSE;
		
		for (i = 0; i < pda->dllIndex; i++)
		{
			if (_tcsicmp(ModEntry.szModule, pda->DllToken[i]) == 0)
			{
				ch = TRUE;
				break;
			}
		}

		if (ch == FALSE)
			break;
	} while (Module32Next(hModSnap, &ModEntry));

	//MessageBox(NULL, ModEntry.szModule, _T("정보"), MB_OK);
	CloseHandle(hModSnap);
	if (ch == TRUE && i == pda->dllIndex - 1)
		return MD_OK;
	return MD_INJECT;
}