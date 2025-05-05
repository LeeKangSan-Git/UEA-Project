#pragma once
#include <iostream>
#include <windows.h>
#include <tchar.h>

using namespace std;

#define TOKEN_SIZE		100

struct TData
{
	HWND hWnd;
	TCHAR* Token[TOKEN_SIZE];
	TCHAR* DllToken[TOKEN_SIZE];
	int index, dllIndex;
	HANDLE hThread[2];
	HANDLE hDvEvent;

	TData()
	{
		memset(this, 0, sizeof(*this));
	}
};
using PTDATA = TData*;

LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI CheckThread(LPVOID lParam);
DWORD WINAPI DllCheck(LPVOID lParam);