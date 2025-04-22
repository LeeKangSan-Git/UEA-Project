#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include "main.h"
#include "SnapModule.h"
#include "config.h"

using namespace std;

DWORD WINAPI CheckThread(LPVOID lParam)
{
	PTDATA pda = (PTDATA)lParam;
	if (pda->Token == nullptr && ConfigToken(pda) != 0)
		return -1;
	int index = 0;
	DWORD TargetId = 0;
	HANDLE HTarget = nullptr;

	while (true)
	{
		if (WaitForSingleObject(pda->hDvEvent, 0) == WAIT_OBJECT_0)
			break;
		if (index == pda->index)
			index = 0;
		TargetId = GetProcId(pda->Token[index]);
		if (TargetId == 0)
			index++;
		else
		{
			//에러 처리를 어떻해 할지 고민해볼것
			HTarget = OpenProcess(PROCESS_TERMINATE, FALSE, TargetId);
			if (HTarget == nullptr)
				return -1;
			if (TerminateProcess(HTarget, 0) == FALSE)
				return -1;
			MessageBox(NULL, _T("디버거 감지"), _T("정보"), MB_OK);
		}
	}
	CloseHandle(HTarget);
	return 0;
}

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPInst, LPTSTR CmdParam, int nCmdShow)
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInst;
	wc.lpszClassName = _T("UEA");
	wc.lpszMenuName = _T("AC");

	TData pd;

	if (!RegisterClass(&wc))
	{
		DWORD error = GetLastError();
		return -1;
	}
	HWND hWnd = CreateWindowEx(0, wc.lpszClassName, _T("UEA"), 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInst, (LPVOID)&pd);
	if (!hWnd)
		return -1;

	MSG msg;
	while (GetMessage(&msg, hWnd, NULL, NULL))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PTDATA pd = (PTDATA)lParam;

	switch (uMsg)
	{
		case WM_CREATE:
		{
			DWORD dwThId = 0;
			ConfigToken(pd);
			int index = 0;
			pd->hThread = CreateThread(NULL, 0, CheckThread, (LPVOID)pd, 0, &dwThId);
			pd->hDvEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			return 0;
		}
		case WM_DESTROY:
			SetEvent(pd->hDvEvent);
			WaitForSingleObject(pd->hThread, INFINITE);
			CloseHandle(pd->hDvEvent);
			CloseHandle(pd->hThread);
			PostQuitMessage(0);
			return 0;
		/*타이머를 이용해 매 시간마다 WndProc에서 보호할 프로그램의 모듈을 검사할지
		아니면 스레드를 하나 더 만들어서 검사할지 고민중*/
	}

	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}