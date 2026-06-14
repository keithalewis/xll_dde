// error.h - Error messaged
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdexcept>

inline LPSTR FormatLastError(DWORD err = GetLastError())
{
	static CHAR buf[256];

	DWORD dw = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, buf, sizeof(buf), NULL);

	return buf;
}

#define THROW_LAST_ERROR(err) do { if (err) throw std::runtime_error(FormatLastError(err)); } while(0);
