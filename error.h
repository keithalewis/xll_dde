// error.h - Error messaged
#pragma once
#include <Windows.h>
#include <stdexcept>

inline LPSTR FormatLastError(DWORD err = GetLastError())
{
	static CHAR buf[256];

	DWORD dw = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, buf, sizeof(buf), NULL);

	return buf;
}

#define CHECK_LAST_ERROR() do { DWORD err = GetLastError(); \
	if (err) throw std::runtime_error(FormatLastError(err)); } while(0);