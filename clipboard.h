// clipboard.h - https://learn.microsoft.com/en-us/windows/win32/dataxchg/clipboard
#pragma once
#include "error.h"

class Clipboard {
	HWND owner;
public:
	// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-openclipboard
	Clipboard(HWND owner = NULL)
		: owner(owner)
	{
		OpenClipboard(owner) || CHECK_LAST_ERROR();
	}
	// https://learn.microsoft.com/en-us/windows/win32/api/Winuser/nf-winuser-closeclipboard
	~Clipboard()
	{
		CloseClipboard() || CHECK_LAST_ERROR();
	}
	BOOL Empty()
	{
		return EmptyClipboard();
	}
	// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setclipboarddata
	HANDLE SetData(UINT fmt, HANDLE h)
	{
		return SetClipboardData(fmt, h);
	}
	HANDLE GetData(UINT fmt)
	{
		return GetClipboardData(fmt);
	}
};