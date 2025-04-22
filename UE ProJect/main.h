#pragma once
#include <iostream>
#include <windows.h>
#include <tchar.h>

using namespace std;

#define TOKEN_SIZE		100

struct TData
{
	TCHAR* Token[TOKEN_SIZE];
	int index;
	HANDLE hThread;
	HANDLE hDvEvent;

	TData()
	{
		memset(this, 0, sizeof(*this));
	}
};
using PTDATA = TData*;

LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI CheckThread(LPVOID lParam);