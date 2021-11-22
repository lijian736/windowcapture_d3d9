#ifndef _H_CAPTURE_UTIL_H_
#define _H_CAPTURE_UTIL_H_

#include <string>
#include <vector>
#include <windows.h>
#include <dwmapi.h>

struct WindowAttributes
{
	HWND handle;
	RECT bounds;
	BOOL layered;
	BOOL visible;
	UINT showCmd;
	std::string className;
	std::string title;

	WindowAttributes()
	{
		bounds = { 0,0,0,0 };
		layered = FALSE;
	}
};


BOOL EnumAllCapturedWindows(std::vector<WindowAttributes>& wins);

BOOL DisplayThumbnail(HWND destHwnd, HWND srcHwnd, HTHUMBNAIL& thumbnail, int left, int top, int right, int bottom);
void StopDisplayThumbnail(HTHUMBNAIL thumbnail);

#endif
