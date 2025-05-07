#pragma once
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <tchar.h>
#include "main.h"

using namespace std;

enum MdCheck { MD_INJECT, MD_OK, MD_ERROR };

DWORD GetProcId(LPCTSTR Name);
DWORD CheckModule(PTDATA pda, DWORD pId);
