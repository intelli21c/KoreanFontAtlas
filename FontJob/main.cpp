// Source - https://stackoverflow.com/questions/45518843/initializing-opengl-without-libraries
// Posted by Celestia, modified by community. See post 'Timeline' for change history
// Retrieved 2025-11-07, License - CC BY-SA 4.0

// Sample code showing how to create a modern OpenGL window and rendering context on Win32.

#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <locale>

#include <windows.h>
#include <windowsx.h>
#include <timeapi.h>

#include "glad/gl.h"
#include "glad/wgl.h"

#include "initopengl.h"

void resize(int, int);
void kbdin(int, bool);
void mousein(int*, int*);
void render();
void update(int);
void timedupdate(int);
void prepare();

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "GLU32.lib")



HDC gldc;

int last = 0;
int tupdateresol = 1000 / 50; // 50fps = 20ms
int tupdateacc = 0;

void updatewrap()
{
	//static int last = 0;
	int now = timeGetTime();
	tupdateacc += now - last;
	while (tupdateacc > tupdateresol)
	{
		//maybe check for max N of followup cycle to prevent stalling on here
		timedupdate(tupdateresol);
		tupdateacc -= tupdateresol;
	}
	update(now - last);
	last = now;
	render();
	SwapBuffers(gldc);
	glFinish();


	///below is fps calculation
	static float framesPerSecond = 0.0f;
	static int fps;
	static float lastTime = 0.0f;
	float currentTime = GetTickCount() * 0.001f;
	++framesPerSecond;
	//printf("Current Frames Per Second: %d\n\n", fps);
	if (currentTime - lastTime > 1.0f)
	{
		lastTime = currentTime;
		fps = (int)framesPerSecond;
		framesPerSecond = 0;
	}
}

void movingtimer(HWND unnamedParam1, UINT unnamedParam2, UINT_PTR unnamedParam3, DWORD unnamedParam4)
{
	updatewrap();
}

bool capture_mouse = false;
bool winmoving = false;

static LRESULT CALLBACK window_callback(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	RECT client_rect;
	PAINTSTRUCT ps;
	HDC dc;
	static UINT_PTR movingtimerid = NULL;

	switch (msg) {
	case WM_CREATE:
		break;
	case WM_WINDOWPOSCHANGED: // Window was resized or moved
	{
		GetClientRect(window, &client_rect);
		int width = client_rect.right - client_rect.left;
		int height = client_rect.bottom - client_rect.top;
		glViewport(0, 0, width, height);
		InvalidateRect(window, NULL, TRUE);
		resize(width, height);
	}
	break;
	case WM_ENTERSIZEMOVE:
		winmoving = true;
		movingtimerid = SetTimer(window, 1, 1, movingtimer); // ~60 FPS
		break;
	case WM_EXITSIZEMOVE:
		winmoving = false;
		KillTimer(window, movingtimerid);
		break;
	case WM_SIZING:
	case WM_MOVING:
		//updatewrap();
		break;
	case WM_PAINT:
		dc = BeginPaint(window, &ps);
		EndPaint(window, &ps);
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		KillTimer(window, 1);
		ReleaseDC(window, gldc);
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)
		{
			capture_mouse = false;
		}
		kbdin((int)wparam, true);
		break;
	case WM_KEYUP:
		kbdin((int)wparam, false);
		break;
	case WM_LBUTTONDOWN:
		//capture_mouse = true;
		break;
	case WM_MOUSEMOVE:

	{
		RECT screen;
		POINT cursor;
		POINT center;
		GetCursorPos(&cursor);
		ScreenToClient(window, &cursor);
		GetClientRect(window, &screen);
		center.x = (screen.right + screen.left) / 2;
		center.y = (screen.bottom + screen.top) / 2;
		int dx = cursor.x - center.x;
		int dy = cursor.y - center.y;
		ClientToScreen(window, &center);
		if (capture_mouse)
		{
			mousein(&dx, &dy);
			SetCursorPos(center.x, center.y);
		}
	}

	break;
	default:
		result = DefWindowProcA(window, msg, wparam, lparam);
		break;
	}

	return result;
}

static HWND create_window(HINSTANCE inst)
{
	WNDCLASSA window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	window_class.lpfnWndProc = window_callback;
	window_class.hInstance = inst;
	window_class.hCursor = LoadCursor(0, IDC_ARROW);
	window_class.hbrBackground = 0;
	window_class.lpszClassName = "WGL_window";

	RegisterClassA(&window_class);

	// Specify a desired width and height, then adjust the rect so the window's client area will be
	// that size.
	RECT rect = { 0,0,1024,768 };
	DWORD window_style = WS_OVERLAPPEDWINDOW;
	AdjustWindowRect(&rect, window_style, false);

	HWND window = CreateWindowExA(
		0,
		window_class.lpszClassName,
		"OpenGL",
		window_style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		0,
		0,
		inst,
		0);

	return window;
}

int main(int argc, char* argv[])
{
	std::locale::global(std::locale("kor"));
	HWND window = create_window(NULL);
	gldc = GetDC(window);
	HGLRC glrc = init_opengl(gldc);
	gladLoaderLoadGL();
	gladLoaderLoadWGL(gldc);
	ShowWindow(window, SW_RESTORE);
	UpdateWindow(window);
	wglSwapIntervalEXT(-1);
	if (timeBeginPeriod(4) != TIMERR_NOERROR)
	{
		puts("Scheduler timer resolution request failed.");
	}
	last = timeGetTime();
	prepare();
	bool running = true;
	while (running) {
		if (GetActiveWindow() != window)
		{
			capture_mouse = false;
		}
		MSG msg;
		while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				running = false;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessageA(&msg);
			}
		}
		updatewrap();
	}

	return 0;
}
