#pragma once
#ifndef INITOPENGL_H
#define INITOPENGL_H
#include <windows.h>
#include "glad/gl.h"
#include "glad/wgl.h"

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext, const int* attribList);

wglCreateContextAttribsARB_type* wglCreateContextAttribsARB;



#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);

wglChoosePixelFormatARB_type* wglChoosePixelFormatARB;



#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B



static void init_opengl_extensions(void)
{
	WNDCLASSA window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	window_class.lpfnWndProc = DefWindowProcA;
	window_class.hInstance = GetModuleHandle(0);
	window_class.lpszClassName = "Dummy_WGL_Window";
	RegisterClassA(&window_class);

	HWND dummy_window = CreateWindowExA(
		0,
		window_class.lpszClassName,
		"Dummy OpenGL Window",
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		window_class.hInstance,
		0);

	HDC dummy_dc = GetDC(dummy_window);

	PIXELFORMATDESCRIPTOR pfd;
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.cColorBits = 32;
	pfd.cAlphaBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;

	int pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
	SetPixelFormat(dummy_dc, pixel_format, &pfd);

	HGLRC dummy_context = wglCreateContext(dummy_dc);
	wglMakeCurrent(dummy_dc, dummy_context);
	wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress("wglCreateContextAttribsARB");
	wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress("wglChoosePixelFormatARB");

	wglMakeCurrent(dummy_dc, 0);
	wglDeleteContext(dummy_context);
	ReleaseDC(dummy_window, dummy_dc);
	DestroyWindow(dummy_window);
}

static HGLRC init_opengl(HDC real_dc)
{
	init_opengl_extensions();
	int pixel_format_attribs[] = {
		WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
		WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,         32,
		WGL_DEPTH_BITS_ARB,         24,
		WGL_STENCIL_BITS_ARB,       8,
		0
	};
	int pixel_format;
	UINT num_formats;
	wglChoosePixelFormatARB(real_dc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
	PIXELFORMATDESCRIPTOR pfd;
	DescribePixelFormat(real_dc, pixel_format, sizeof(pfd), &pfd);
	SetPixelFormat(real_dc, pixel_format, &pfd);
	int gl33_attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0,
	};
	HGLRC gl33_context = wglCreateContextAttribsARB(real_dc, 0, gl33_attribs);
	wglMakeCurrent(real_dc, gl33_context);
	return gl33_context;
}

#endif // !INITOPENGL_H