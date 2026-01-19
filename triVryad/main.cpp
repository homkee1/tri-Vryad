#include <windows.h>
#include "Game.h"
#include "Constants.h"
#include "GameObserver.h"
#pragma comment(lib, "gdiplus.lib")

Game game;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	switch (message) {
	case WM_CREATE:
		game.InitializeGrid();
		SetTimer(hWnd, 1, 16, NULL);
		break;

	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		int windowWidth = clientRect.right - clientRect.left;
		int windowHeight = clientRect.bottom - clientRect.top;

		HDC hdcMem = CreateCompatibleDC(hdc);
		HBITMAP hbmMem = CreateCompatibleBitmap(hdc, windowWidth, windowHeight);
		SelectObject(hdcMem, hbmMem);

		game.DrawGrid(hdcMem, windowWidth, windowHeight);

		BitBlt(hdc, 0, 0, windowWidth, windowHeight, hdcMem, 0, 0, SRCCOPY);

		DeleteObject(hbmMem);
		DeleteDC(hdcMem);

		EndPaint(hWnd, &ps);
		break;
	}

	case WM_TIMER: {
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		int windowWidth = clientRect.right - clientRect.left;
		int windowHeight = clientRect.bottom - clientRect.top;

		game.Update(windowWidth, windowHeight);

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_MOUSEMOVE: {
		if (!game.IsFirstGemSelected()) {
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		else {
			SetCursor(LoadCursor(NULL, IDC_HAND));
		}
		break;
	}

	case WM_LBUTTONDOWN: {
		int pixelX = LOWORD(lParam);
		int pixelY = HIWORD(lParam);

		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		int windowWidth = clientRect.right - clientRect.left;
		int windowHeight = clientRect.bottom - clientRect.top;

		int cellWidth = windowWidth / GRID_SIZE;
		int cellHeight = windowHeight / GRID_SIZE;

		game.HandleClick(pixelX, pixelY, cellWidth, cellHeight);
		break;
	}

	case WM_DESTROY:
		KillTimer(hWnd, 1);
		game.Cleanup();
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc = {
	sizeof(WNDCLASSEX),
	CS_HREDRAW | CS_VREDRAW,
	WndProc,
	0, 0, hInstance, NULL, NULL, NULL, NULL,
	L"GemClass", NULL
	};
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClassEx(&wc);

	int windowWidth = 416;
	int windowHeight = 439;
	HWND hWnd = CreateWindowEx(
		0,
		L"GemClass",
		L"tri v ryad",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowWidth, windowHeight,
		NULL, NULL, hInstance, NULL
	);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	GameObserver observer(&game);
	game.AddObserver(&observer);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}