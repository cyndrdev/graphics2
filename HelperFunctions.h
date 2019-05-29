#pragma once
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <iomanip>
#include <fstream>

#define HISTORY_SIZE 1000;
static COORD p;

// Various helper functions
inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}

inline void ConsoleCleanup(FILE* cin, FILE* cout, FILE* cerr) {
	// sync stream functions
	std::ios::sync_with_stdio(true);

	// redirect the crt handles
	freopen_s(&cin, "CONIN$", "r", stdin);
	freopen_s(&cout, "CONOUT$", "w", stdout);
	freopen_s(&cerr, "CONOUT$", "w", stderr);

	// clear stream state
	std::wcout.clear();
	std::cout.clear();
	std::wcerr.clear();
	std::cerr.clear();
	std::wcin.clear();
	std::cin.clear();
}

// redirect io output to console
inline void InitDebugConsole()
{
	// create the console
	AllocConsole();
	std::wstring title = L"ｏｐｅｎ　ｔｈｅｉｒ　ｍｉｎｄｓ　ゥきサ";
	SetConsoleTitle(title.c_str());

	// get stdout handle
	HANDLE ConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	int SystemOutput = _open_osfhandle(intptr_t(ConsoleOutput), _O_TEXT);
	FILE* COutputHandle = _fdopen(SystemOutput, "w");

	// get stderr handle
	HANDLE ConsoleError = GetStdHandle(STD_ERROR_HANDLE);
	int SystemError = _open_osfhandle(intptr_t(ConsoleError), _O_TEXT);
	FILE* CErrorHandle = _fdopen(SystemError, "w");

	// get stdin handle
	HANDLE ConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
	int SystemInput = _open_osfhandle(intptr_t(ConsoleInput), _O_TEXT);
	FILE* CInputHandle = _fdopen(SystemInput, "r");

	ConsoleCleanup(CInputHandle, COutputHandle, CErrorHandle);
}

// === console graphics helpers === //
inline void StartStatbar() {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo;
	GetConsoleScreenBufferInfo(consoleHandle, &lpConsoleScreenBufferInfo);
	p = lpConsoleScreenBufferInfo.dwCursorPosition;
}

inline void UpdateStatbar(int perc) {
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), p);

	std::cout << std::setfill('0') << std::setw(3) << perc;
	std::cout << "% [";

	int m = (int)roundf(perc / 5.0f);

	for (int i = 0; i < 20; i++) {
		std::cout << ((i < m) ? "#" : " ");
	}
	std::cout << "]" << std::endl;
}