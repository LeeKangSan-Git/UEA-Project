#pragma once
#include <iostream>
#include <windows.h>
#include <tchar.h>
#include "SnapModule.h"
#include "main.h"

#define CONFIG_FILE "config.txt"

using namespace std;

void TrimSpaces(TCHAR*& str);
DWORD ConfigToken(PTDATA pda);