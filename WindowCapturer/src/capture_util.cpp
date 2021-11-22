#include "capture_util.h"
#include <dwmapi.h>

#pragma comment(lib, "Dwmapi.lib")

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	std::vector<WindowAttributes>* vecs = (std::vector<WindowAttributes>*)lParam;

	LONG_PTR style;
	LONG_PTR exstyle;

	style = ::GetWindowLongPtr(hwnd, GWL_STYLE);
	exstyle = ::GetWindowLongPtr(hwnd, GWL_EXSTYLE);

	if (style & WS_CHILD)
	{
		//if the window is child window, continue to enum
		return TRUE;
	}

	if (exstyle & WS_EX_TOOLWINDOW)
	{
		//if the window is tool window, continue to enum
		return TRUE;
	}

	int cloaked;
	if (S_OK == DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked)) && cloaked)
	{
		//if the window is cloaked, continue to enum
		return TRUE;
	}

	DWORD id;
	GetWindowThreadProcessId(hwnd, &id);
	if (id == GetCurrentProcessId())
	{
		//if the window is the current app's window, then continue to enum
		return TRUE;
	}

	WindowAttributes window;
	window.handle = hwnd;

	RECT frameBounds;
	frameBounds.left = frameBounds.right = frameBounds.bottom = frameBounds.top = 0;
	if (S_OK == DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &frameBounds, sizeof(frameBounds)))
	{
		window.bounds = frameBounds;
	}

	window.layered = (exstyle & WS_EX_LAYERED) != 0;

	char szTitle[512] = { 0 };
	::GetWindowTextA(hwnd, (LPSTR)szTitle, sizeof(szTitle));
	window.title = szTitle;
	//StringCbCopyA(window.title, sizeof(window.title), szTitle);

	WINDOWPLACEMENT placement;
	placement.length = sizeof(WINDOWPLACEMENT);
	placement.flags = 0;
	if (GetWindowPlacement(hwnd, &placement))
	{
		window.showCmd = placement.showCmd;
	}

	//get the icon
	HICON hIcon = (HICON)GetClassLong(hwnd, GCL_HICON);

	char szText[1024] = { 0 };
	GetClassNameA(hwnd, szText, 1024);
	window.className = szText;

	window.visible = (style & WS_VISIBLE) != 0; // IsWindowVisible(hwnd);
	if (window.visible && window.bounds.right - window.bounds.left > 0 && window.title.length() > 0)
	{
		vecs->push_back(window);
	}

	return TRUE;
}


BOOL EnumAllCapturedWindows(std::vector<WindowAttributes>& wins)
{
	wins.clear();
	return EnumWindows(EnumWindowsProc, (LPARAM)&wins);
}

BOOL DisplayThumbnail(HWND destHwnd, HWND srcHwnd, HTHUMBNAIL& thumbnail, int left, int top, int right, int bottom)
{
	HRESULT hr = S_OK;

	DwmUnregisterThumbnail(thumbnail);
	thumbnail = NULL;

	// Register the thumbnail
	hr = DwmRegisterThumbnail(destHwnd, srcHwnd, &thumbnail);
	if (SUCCEEDED(hr))
	{
		// Specify the destination rectangle size
		RECT dest;

		SIZE size;
		DwmQueryThumbnailSourceSize(thumbnail, &size);

		dest.left = left;
		dest.top = top;
		dest.right = right;
		dest.bottom = bottom;

		// Set the thumbnail properties for use
		DWM_THUMBNAIL_PROPERTIES dskThumbProps;
		dskThumbProps.dwFlags = DWM_TNP_SOURCECLIENTAREAONLY | DWM_TNP_VISIBLE | DWM_TNP_RECTDESTINATION;
		dskThumbProps.fSourceClientAreaOnly = FALSE;
		dskThumbProps.fVisible = TRUE;
		dskThumbProps.opacity = 255;
		dskThumbProps.rcDestination = dest;

		// Display the thumbnail
		hr = DwmUpdateThumbnailProperties(thumbnail, &dskThumbProps);
		if (SUCCEEDED(hr))
		{
			return TRUE;
		}

		DwmUnregisterThumbnail(thumbnail);
	}
	
	return FALSE;
}

void StopDisplayThumbnail(HTHUMBNAIL thumbnail)
{
	DwmUnregisterThumbnail(thumbnail);
}