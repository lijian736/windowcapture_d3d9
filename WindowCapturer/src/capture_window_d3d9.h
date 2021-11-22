#ifndef _H_D3D9_CAPTURE_SCREEN_H_
#define _H_D3D9_CAPTURE_SCREEN_H_

#include <functional>
#include <windows.h>

#include <d3d9.h>

//the callback function
typedef std::function< void(unsigned char* data, unsigned long dataLength,
	unsigned int renderWidth, unsigned int renderHeight) > OnD3DScreenDraw;

/**
* capture screen class by Direct3D 9
*/
class D3D9CaptureScreen
{
public:
	D3D9CaptureScreen();
	virtual ~D3D9CaptureScreen();

	BOOL init();
	BOOL un_init();

	BOOL render_loop();
	void set_draw_callback(OnD3DScreenDraw func);

private:
	//is the class initialized
	BOOL m_initialized;
	//the draw callback function
	OnD3DScreenDraw m_draw_fun;

	//the dirext3d 9
	IDirect3D9Ex* m_d3d9;
	//the direct3d device
	IDirect3DDevice9Ex* m_device;
	//the direct3d surface
	IDirect3DSurface9* m_surface;
	//the data buffer
	BYTE* m_buffer;
	//the buffer length
	ULONG m_buf_len;
	//the screen width
	UINT m_screen_width;
	//the screen height
	UINT m_screen_height;
};
#endif