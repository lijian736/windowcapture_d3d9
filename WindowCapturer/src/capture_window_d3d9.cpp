#include "capture_window_d3d9.h"

#include <dwmapi.h>

namespace 
{
	constexpr int BYTES_PER_PIXEL = 4;
	typedef HRESULT(WINAPI *D3D9CreateEx)(UINT, IDirect3D9Ex **);
}

D3D9CaptureScreen::D3D9CaptureScreen()
{
	m_initialized = FALSE;
	m_d3d9 = NULL;
	m_device = NULL;
	m_surface = NULL;
	m_screen_width = 0;
	m_screen_height = 0;
	m_buffer = NULL;
	m_buf_len = 0;
}

D3D9CaptureScreen::~D3D9CaptureScreen()
{
	un_init();
}

BOOL D3D9CaptureScreen::init()
{
	if (m_initialized)
	{
		return TRUE;
	}

	un_init();

	m_initialized = FALSE;

	HRESULT hr;
	D3D9CreateEx d3d9CreateFun;
	D3DPRESENT_PARAMETERS presentParams;
	D3DCAPS9 caps;
	D3DDISPLAYMODE d3ddm;
	DWORD vp;
	HMODULE module;
	HWND screenHwnd;

	module = LoadLibraryA("d3d9.dll");
	if (!module)
	{
		return FALSE;
	}

	d3d9CreateFun = (D3D9CreateEx)GetProcAddress(module, "Direct3DCreate9Ex");
	if (!d3d9CreateFun)
	{
		goto exitFlag;
	}

	hr = d3d9CreateFun(D3D_SDK_VERSION, &m_d3d9);
	if (FAILED(hr))
	{
		goto exitFlag;
	}

	vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	hr = m_d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
	if (SUCCEEDED(hr) && caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}

	if (FAILED(m_d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
	{
		goto exitFlag;
	}

	screenHwnd = GetDesktopWindow();
	m_screen_width = d3ddm.Width;
	m_screen_height = d3ddm.Height;

	ZeroMemory(&presentParams, sizeof(D3DPRESENT_PARAMETERS));
	presentParams.Windowed = TRUE;
	presentParams.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParams.BackBufferFormat = D3DFMT_A8R8G8B8; // d3ddm.Format;
	presentParams.BackBufferWidth = 2;  //d3ddm.Width;
	presentParams.BackBufferHeight = 2;  //d3ddm.Height;
	presentParams.BackBufferCount = 1;
	presentParams.hDeviceWindow = screenHwnd;
	presentParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	presentParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	hr = m_d3d9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
		screenHwnd, vp, &presentParams, NULL, &m_device);
	if (FAILED(hr))
	{
		goto exitFlag;
	}

	hr = m_device->CreateOffscreenPlainSurface(m_screen_width, m_screen_height,
		D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &m_surface, NULL);
	if (FAILED(hr))
	{
		goto exitFlag;
	}

	UINT lineBytes = m_screen_width * BYTES_PER_PIXEL;
	UINT lineStride = (lineBytes + 3) / 4 * 4;
	m_buf_len = lineStride * m_screen_height;
	m_buffer = new BYTE[m_buf_len];
	if (!m_buffer)
	{
		goto exitFlag;
	}


	m_initialized = TRUE;
	return TRUE;

exitFlag:
	un_init();
	return FALSE;
}

BOOL D3D9CaptureScreen::un_init()
{
	m_initialized = FALSE;

	if (m_buffer)
	{
		delete[] m_buffer;
		m_buffer = NULL;
		m_buf_len = 0;
	}

	if (m_surface)
	{
		m_surface->Release();
		m_surface = NULL;
	}
	if (m_device)
	{
		m_device->Release();
		m_device = NULL;
	}
	if (m_d3d9)
	{
		m_d3d9->Release();
		m_d3d9 = NULL;
	}

	m_screen_width = 0;
	m_screen_height = 0;
	return TRUE;
}

BOOL D3D9CaptureScreen::render_loop()
{
	if (!m_initialized)
	{
		return FALSE;
	}

	HRESULT hr;
	if (m_device && m_surface)
	{
		hr = m_device->GetFrontBufferData(0, m_surface);
		if (FAILED(hr))
		{
			return FALSE;
		}

		D3DLOCKED_RECT lockedRect;
		hr = m_surface->LockRect(&lockedRect, NULL, D3DLOCK_NO_DIRTY_UPDATE |
			D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY);
		if (FAILED(hr))
		{
			return FALSE;
		}

		for (UINT i = 0; i < m_screen_height; i++)
		{
			memcpy((BYTE*)m_buffer + i * m_screen_width * BYTES_PER_PIXEL,
				(BYTE*)lockedRect.pBits + i * lockedRect.Pitch,
				m_screen_width * BYTES_PER_PIXEL);
		}
		m_surface->UnlockRect();

		m_draw_fun(m_buffer, m_buf_len, m_screen_width, m_screen_height);
	}

	return TRUE;
}

void D3D9CaptureScreen::set_draw_callback(OnD3DScreenDraw fun)
{
	this->m_draw_fun = fun;
}