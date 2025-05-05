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
	if (pda->index == 0)
	{
		if (ConfigToken(pda) != 0)
			return -1;
	}
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
			TerminateProcess(HTarget, 0);
			MessageBox(NULL, _T("디버거 감지"), _T("정보"), MB_ICONHAND);
			CloseHandle(HTarget);
		}
	}
	return 0;
}

DWORD WINAPI DllCheck(LPVOID lParam)
{
	PTDATA pda = (PTDATA)lParam;
	DWORD hTarget;
	int index = 0, check = 0;
	
	while (true)
	{
		do
		{
			hTarget = GetProcId(_T("notepad.exe"));
		} while (hTarget == 0);

		if (index == pda->dllIndex)
			index = 0;
		check = 0;
		check = CheckModule(pda->DllToken[index], hTarget);
		if (check == 1)
		{
			index++;
			continue;
		}
		else if (check == 2)
			continue;
		else
		{
			HANDLE hT = OpenProcess(PROCESS_TERMINATE, FALSE, hTarget);
			TerminateProcess(hT, 0);
			CloseHandle(hT);
			MessageBox(NULL, _T("인젝션 감지"), _T("경고"), MB_ICONHAND);
			SetEvent(pda->hDvEvent);
			PostMessage(pda->hWnd, WM_DESTROY, 0, 0);
			break;
		}
	}
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
	pd.hWnd = hWnd;

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

	if (uMsg == WM_NCCREATE)
	{
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		pd = (PTDATA)cs->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pd);
	}
	else
		pd = (PTDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (uMsg)
	{
		case WM_CREATE:
		{
			DWORD dwThId = 0;
			pd->hDvEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			ConfigToken(pd);
			int index = 0;
			//while (index != pd->index)
			//	MessageBox(NULL, pd->Token[index++], _T("정보"), MB_OK);
			pd->hThread[0] = CreateThread(NULL, 0, CheckThread, (LPVOID)pd, 0, &dwThId);
			pd->hThread[1] = CreateThread(NULL, 0, DllCheck, (LPVOID)pd, 0, &dwThId);
			return 0;
		}
		case WM_DESTROY:
			WaitForMultipleObjects(2, pd->hThread, TRUE, INFINITE);
			CloseHandle(pd->hDvEvent);
			for (int i = 0; i < 2; i++)
				CloseHandle(pd->hThread[i]);
			PostQuitMessage(0);
			return 0;
			/*타이머를 이용해 매 시간마다 WndProc에서 보호할 프로그램의 모듈을 검사할지
			아니면 스레드를 하나 더 만들어서 검사할지 고민중*/
	}

	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}