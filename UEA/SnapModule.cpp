#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include "main.h"

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

DWORD CheckModule(LPCTSTR Name, DWORD pId)
{
	HANDLE hModSnap;
	MODULEENTRY32 ModEntry;

	hModSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pId);
	if (hModSnap == INVALID_HANDLE_VALUE)
		return 0;

	ModEntry.dwSize = sizeof(MODULEENTRY32);
	Module32First(hModSnap, &ModEntry);
	do
	{
		if (!_tcsicmp(ModEntry.szModule, Name))
			break;
	} while (Module32Next(hModSnap, &ModEntry));

	CloseHandle(hModSnap);
	return 1;
}