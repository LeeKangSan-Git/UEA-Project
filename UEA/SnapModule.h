#pragma once
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <tchar.h>
#include "main.h"

using namespace std;

DWORD GetProcId(LPCTSTR Name);
DWORD CheckModule(LPCTSTR Name, DWORD pId);
