#include <windows.h>
#include <string>
#include <shlobj.h>
#include <shobjidl.h>
#include <thread>
#include <future>
#include <memory>
#include "Game.h"
#include "Constants.h"
#include "GameObserver.h"

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib") 

#define ID_CHECK_SAVE      2010
#define WM_RESET_GAME      (WM_USER + 1)
#define WM_FOLDER_SELECTED (WM_USER + 2)

#define ID_MENU_SETTINGS   1001
#define ID_MENU_EXIT       1002
#define ID_MENU_RESTART    1003

#define ID_EDIT_GRID_SIZE  2001
#define ID_EDIT_CELL_SIZE  2002
#define ID_EDIT_GEM_TYPES  2003
#define ID_EDIT_SPEED      2004
#define ID_EDIT_PATH       2005
#define ID_BTN_BROWSE      2006
#define ID_COMBO_QUALITY   2007
#define ID_EDIT_IMG_SIZE   2008
#define ID_BTN_SAVE        2009

struct NewSettings {
	int gridSize;
	int cellSize;
	int gemTypesCount;
	double speed;
	std::wstring pngFolder;
	int quality;
	int gemImageSize;
	bool saveSettings;
};

bool SAVE_SETTINGS = false;

HWND hEditGrid = NULL, hEditCell = NULL, hEditTypes = NULL, hEditSpeed = NULL, hEditPath = NULL, hComboQuality = NULL, hEditImgSize = NULL;

void TriggerAsyncBrowseFolder(HWND hSettingsWnd) {
	HWND hBtnBrowse = GetDlgItem(hSettingsWnd, ID_BTN_BROWSE);
	if (hBtnBrowse) {
		EnableWindow(hBtnBrowse, FALSE);
	}

	std::thread([hSettingsWnd]() {
		wchar_t path[MAX_PATH] = { 0 };
		BROWSEINFOW bi = { 0 };
		bi.hwndOwner = hSettingsWnd;
		bi.lpszTitle = L"выберите папку с картинками:";
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NONEWFOLDERBUTTON;

		LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
		if (pidl != NULL) {
			SHGetPathFromIDListW(pidl, path);
			CoTaskMemFree(pidl);

			std::wstring* pPath = new std::wstring(path);
			if (!PostMessageW(hSettingsWnd, WM_FOLDER_SELECTED, 0, (LPARAM)pPath)) {
				delete pPath;
			}
		}
		else {
			PostMessageW(hSettingsWnd, WM_FOLDER_SELECTED, 0, 0);
		}
		}).detach();
}
void SaveSettingsToRegistry() {
	HKEY hKey;
	if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\TriVRyad", 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {

		DWORD dwSave = SAVE_SETTINGS ? 1 : 0;
		RegSetValueExW(hKey, L"SaveSettings", 0, REG_DWORD, (BYTE*)&dwSave, sizeof(dwSave));

		if (SAVE_SETTINGS) {
			DWORD dwGridSize = GRID_SIZE;
			DWORD dwCellSize = CELL_SIZE;
			DWORD dwGemTypes = GEM_TYPES_COUNT;
			DWORD dwQuality = RENDERING_QUALITY;
			DWORD dwImgSize = GEM_IMAGE_SIZE;

			RegSetValueExW(hKey, L"GridSize", 0, REG_DWORD, (BYTE*)&dwGridSize, sizeof(dwGridSize));
			RegSetValueExW(hKey, L"CellSize", 0, REG_DWORD, (BYTE*)&dwCellSize, sizeof(dwCellSize));
			RegSetValueExW(hKey, L"GemTypes", 0, REG_DWORD, (BYTE*)&dwGemTypes, sizeof(dwGemTypes));
			RegSetValueExW(hKey, L"RenderingQuality", 0, REG_DWORD, (BYTE*)&dwQuality, sizeof(dwQuality));
			RegSetValueExW(hKey, L"GemImageSize", 0, REG_DWORD, (BYTE*)&dwImgSize, sizeof(dwImgSize));
			RegSetValueExW(hKey, L"Speed", 0, REG_BINARY, (BYTE*)&SpeedAnimationA4ChangePos, sizeof(SpeedAnimationA4ChangePos));
			RegSetValueExW(hKey, L"PngFolder", 0, REG_SZ, (BYTE*)PNG_FOLDER_PATH.c_str(), (DWORD)((PNG_FOLDER_PATH.length() + 1) * sizeof(wchar_t)));
		}
		RegCloseKey(hKey);
	}
}

void LoadSettingsFromRegistry() {
	HKEY hKey;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\TriVRyad", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		DWORD dwSave = 0;
		DWORD dwSize = sizeof(dwSave);
		if (RegQueryValueExW(hKey, L"SaveSettings", NULL, NULL, (BYTE*)&dwSave, &dwSize) == ERROR_SUCCESS) {
			SAVE_SETTINGS = (dwSave != 0);
		}

		if (SAVE_SETTINGS) {
			DWORD dwValue = 0;
			dwSize = sizeof(dwValue);
			if (RegQueryValueExW(hKey, L"GridSize", NULL, NULL, (BYTE*)&dwValue, &dwSize) == ERROR_SUCCESS) {
				GRID_SIZE = dwValue;
			}
			dwSize = sizeof(dwValue);
			if (RegQueryValueExW(hKey, L"CellSize", NULL, NULL, (BYTE*)&dwValue, &dwSize) == ERROR_SUCCESS) {
				CELL_SIZE = dwValue;
			}
			dwSize = sizeof(dwValue);
			if (RegQueryValueExW(hKey, L"GemTypes", NULL, NULL, (BYTE*)&dwValue, &dwSize) == ERROR_SUCCESS) {
				GEM_TYPES_COUNT = dwValue;
			}
			dwSize = sizeof(dwValue);
			if (RegQueryValueExW(hKey, L"RenderingQuality", NULL, NULL, (BYTE*)&dwValue, &dwSize) == ERROR_SUCCESS) {
				RENDERING_QUALITY = dwValue;
			}
			dwSize = sizeof(dwValue);
			if (RegQueryValueExW(hKey, L"GemImageSize", NULL, NULL, (BYTE*)&dwValue, &dwSize) == ERROR_SUCCESS) {
				GEM_IMAGE_SIZE = dwValue;
			}

			double dSpeed = 0.0;
			dwSize = sizeof(dSpeed);
			if (RegQueryValueExW(hKey, L"Speed", NULL, NULL, (BYTE*)&dSpeed, &dwSize) == ERROR_SUCCESS) {
				SpeedAnimationA4ChangePos = dSpeed;
			}

			wchar_t szPath[MAX_PATH] = { 0 };
			dwSize = sizeof(szPath);
			if (RegQueryValueExW(hKey, L"PngFolder", NULL, NULL, (BYTE*)szPath, &dwSize) == ERROR_SUCCESS) {
				PNG_FOLDER_PATH = szPath;
			}
		}
		RegCloseKey(hKey);
	}
}

LRESULT CALLBACK SettingsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE: {
		int y = 15;
		CreateWindowW(L"STATIC", L"Размер сетки:", WS_VISIBLE | WS_CHILD, 15, y, 180, 20, hWnd, NULL, NULL, NULL);
		hEditGrid = CreateWindowW(L"EDIT", std::to_wstring(GRID_SIZE).c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 210, y, 180, 20, hWnd, (HMENU)ID_EDIT_GRID_SIZE, NULL, NULL);

		y += 35;
		CreateWindowW(L"STATIC", L"Размер гема:", WS_VISIBLE | WS_CHILD, 15, y, 180, 20, hWnd, NULL, NULL, NULL);
		hEditCell = CreateWindowW(L"EDIT", std::to_wstring(CELL_SIZE).c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 210, y, 180, 20, hWnd, (HMENU)ID_EDIT_CELL_SIZE, NULL, NULL);

		y += 35;
		CreateWindowW(L"STATIC", L"Типов гемов:", WS_VISIBLE | WS_CHILD, 15, y, 180, 20, hWnd, NULL, NULL, NULL);
		hEditTypes = CreateWindowW(L"EDIT", std::to_wstring(GEM_TYPES_COUNT).c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 210, y, 180, 20, hWnd, (HMENU)ID_EDIT_GEM_TYPES, NULL, NULL);

		y += 35;
		CreateWindowW(L"STATIC", L"Скорость анимации:", WS_VISIBLE | WS_CHILD, 15, y, 180, 20, hWnd, NULL, NULL, NULL);
		hEditSpeed = CreateWindowW(L"EDIT", std::to_wstring((int)SpeedAnimationA4ChangePos).c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 210, y, 180, 20, hWnd, (HMENU)ID_EDIT_SPEED, NULL, NULL);

		y += 35;
		CreateWindowW(L"STATIC", L"Путь к картинкам (PNG):", WS_VISIBLE | WS_CHILD, 15, y, 180, 20, hWnd, NULL, NULL, NULL);
		hEditPath = CreateWindowW(L"EDIT", PNG_FOLDER_PATH.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 210, y, 115, 20, hWnd, (HMENU)ID_EDIT_PATH, NULL, NULL);
		CreateWindowW(L"BUTTON", L"Обзор", WS_VISIBLE | WS_CHILD, 330, y - 2, 60, 24, hWnd, (HMENU)ID_BTN_BROWSE, NULL, NULL);

		y += 35;
		CreateWindowW(L"STATIC", L"Качество отрисовки:", WS_VISIBLE | WS_CHILD, 15, y, 180, 20, hWnd, NULL, NULL, NULL);
		hComboQuality = CreateWindowW(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 210, y, 180, 100, hWnd, (HMENU)ID_COMBO_QUALITY, NULL, NULL);
		SendMessageW(hComboQuality, CB_ADDSTRING, 0, (LPARAM)L"Низкое");
		SendMessageW(hComboQuality, CB_ADDSTRING, 0, (LPARAM)L"Среднее");
		SendMessageW(hComboQuality, CB_ADDSTRING, 0, (LPARAM)L"Высокое");
		SendMessageW(hComboQuality, CB_SETCURSEL, RENDERING_QUALITY, 0);

		y += 35;
		CreateWindowW(L"STATIC", L"Сжатие текстур:", WS_VISIBLE | WS_CHILD, 15, y, 180, 20, hWnd, NULL, NULL, NULL);
		hEditImgSize = CreateWindowW(L"EDIT", std::to_wstring(GEM_IMAGE_SIZE).c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 210, y, 180, 20, hWnd, (HMENU)ID_EDIT_IMG_SIZE, NULL, NULL);
		y += 35;
		HWND hCheckSave = CreateWindowW(L"BUTTON", L"сохранение настроек",
			WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
			15, y, 380, 20, hWnd, (HMENU)ID_CHECK_SAVE, NULL, NULL);

		if (SAVE_SETTINGS) {
			SendMessageW(hCheckSave, BM_SETCHECK, BST_CHECKED, 0);
		}

		y += 45;
		CreateWindowW(L"BUTTON", L"Применить", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 150, y, 120, 30, hWnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
		break;
	}
	case WM_COMMAND: {
		if (LOWORD(wParam) == ID_BTN_BROWSE) {
			TriggerAsyncBrowseFolder(hWnd);
		}
		else if (LOWORD(wParam) == ID_BTN_SAVE) {
			wchar_t gBuf[16], cBuf[16], tBuf[16], sBuf[16], pBuf[MAX_PATH], imgSzBuf[16];
			GetWindowTextW(hEditGrid, gBuf, 16);
			GetWindowTextW(hEditCell, cBuf, 16);
			GetWindowTextW(hEditTypes, tBuf, 16);
			GetWindowTextW(hEditSpeed, sBuf, 16);
			GetWindowTextW(hEditPath, pBuf, MAX_PATH);
			GetWindowTextW(hEditImgSize, imgSzBuf, 16);
			int qIndex = (int)SendMessageW(hComboQuality, CB_GETCURSEL, 0, 0);

			int nGrid = _wtoi(gBuf);
			int nCell = _wtoi(cBuf);
			int nTypes = _wtoi(tBuf);
			double nSpeed = _wtof(sBuf);
			int nImgSize = _wtoi(imgSzBuf);

			HWND hCheckSave = GetDlgItem(hWnd, ID_CHECK_SAVE);
			bool bSave = (SendMessageW(hCheckSave, BM_GETCHECK, 0, 0) == BST_CHECKED);

			if (nGrid < 1 || nGrid > 99999) {
				MessageBoxW(hWnd, L"размер сетки должен быть в диапазоне от 1 до 99999!", L"Ошибка", MB_ICONERROR);
				break;
			}
			if (nCell < 1 || nCell > 99999) {
				MessageBoxW(hWnd, L"размр гема должен быть в диапазоне от 1 до 99999!", L"Ошибка", MB_ICONERROR);
				break;
			}
			if (nTypes < 1 || nTypes > 99999) {
				MessageBoxW(hWnd, L"количество типов гемов должно быть в диапазоне от 1 до 99999!", L"Ошибка", MB_ICONERROR);
				break;
			}
			if (nSpeed < 0.0 || nSpeed > 99999.0) {
				MessageBoxW(hWnd, L"скорость анимации должна быть в диапазоне от 0.0 до 99999.0!", L"Ошибка", MB_ICONERROR);
				break;
			}
			if (nImgSize < 2 || nImgSize > 65536) {
				MessageBoxW(hWnd, L"разрешение текстур должно быть в диапазоне от 2 до 65 536 пикселей!", L"Ошибка", MB_ICONERROR);
				break;
			}

			NewSettings* pSettings = new NewSettings();
			pSettings->gridSize = nGrid;
			pSettings->cellSize = nCell;
			pSettings->gemTypesCount = nTypes;
			pSettings->speed = nSpeed;
			pSettings->pngFolder = pBuf;
			pSettings->quality = qIndex;
			pSettings->gemImageSize = nImgSize;
			pSettings->saveSettings = bSave;

			HWND hMainWnd = GetWindow(hWnd, GW_OWNER);
			if (hMainWnd) {
				PostMessageW(hMainWnd, WM_RESET_GAME, 0, (LPARAM)pSettings);
			}
			else {
				delete pSettings;
			}
			DestroyWindow(hWnd);
		}
		break;
	}
	case WM_FOLDER_SELECTED: {
		std::wstring* path = (std::wstring*)lParam;
		if (path) {
			SetWindowTextW(hEditPath, path->c_str());
			delete path;
		}
		HWND hBtnBrowse = GetDlgItem(hWnd, ID_BTN_BROWSE);
		if (hBtnBrowse) {
			EnableWindow(hBtnBrowse, TRUE);
		}
		break;
	}
	case WM_DESTROY: {
		HWND hMainWnd = GetWindow(hWnd, GW_OWNER);
		if (hMainWnd) {
			EnableWindow(hMainWnd, TRUE);
			SetForegroundWindow(hMainWnd);
		}
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	Game* game = reinterpret_cast<Game*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message) {
	case WM_NCCREATE: {
		CREATESTRUCT* pcs = reinterpret_cast<CREATESTRUCT*>(lParam);
		Game* g = reinterpret_cast<Game*>(pcs->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(g));
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_CREATE:
		if (game) {
			game->InitializeGrid();
		}
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
		HBITMAP hOldBm = (HBITMAP)SelectObject(hdcMem, hbmMem);

		if (game) {
			game->DrawGrid(hdcMem, windowWidth, windowHeight);
		}

		BitBlt(hdc, 0, 0, windowWidth, windowHeight, hdcMem, 0, 0, SRCCOPY);

		SelectObject(hdcMem, hOldBm);
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

		if (game) {
			game->Update(windowWidth, windowHeight);
		}

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case ID_MENU_RESTART: {
			if (game) {
				game->Reset();
			}
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		}
		case ID_MENU_SETTINGS: {
			HWND hSettingsWnd = CreateWindowExW(
				WS_EX_DLGMODALFRAME,
				L"SettingsClass", L"Настройки игры",
				WS_VISIBLE | WS_SYSMENU | WS_CAPTION,
				CW_USEDEFAULT, CW_USEDEFAULT, 420, 395, 
				hWnd, NULL, GetModuleHandle(NULL), NULL
			);

			EnableWindow(hWnd, FALSE);
			break;
		}
		case ID_MENU_EXIT:
			DestroyWindow(hWnd);
			break;
		}
		break;
	}

	case WM_RESET_GAME: {
		EnableWindow(hWnd, TRUE);
		SetForegroundWindow(hWnd);

		NewSettings* pSettings = (NewSettings*)lParam;
		if (pSettings) {
			bool needsReset = (GRID_SIZE != pSettings->gridSize) ||
				(CELL_SIZE != pSettings->cellSize) ||
				(GEM_TYPES_COUNT != pSettings->gemTypesCount) ||
				(PNG_FOLDER_PATH != pSettings->pngFolder) ||
				(GEM_IMAGE_SIZE != pSettings->gemImageSize);

			GRID_SIZE = pSettings->gridSize;
			CELL_SIZE = pSettings->cellSize;
			GEM_TYPES_COUNT = pSettings->gemTypesCount;
			SpeedAnimationA4ChangePos = pSettings->speed;
			PNG_FOLDER_PATH = pSettings->pngFolder;
			RENDERING_QUALITY = pSettings->quality;
			GEM_IMAGE_SIZE = pSettings->gemImageSize;
			SAVE_SETTINGS = pSettings->saveSettings;

			delete pSettings;

			SaveSettingsToRegistry();

			if (needsReset) {
				if (game) {
					game->Reset();
				}
			}
		}

		InvalidateRect(hWnd, NULL, TRUE);
		break;
	}

	case WM_MOUSEMOVE: {
		if (game) {
			if (!game->IsFirstGemSelected()) {
				SetCursor(LoadCursor(NULL, IDC_ARROW));
			}
			else {
				SetCursor(LoadCursor(NULL, IDC_HAND));
			}
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

		if (game) {
			int cellWidth = windowWidth / GRID_SIZE;
			int cellHeight = windowHeight / GRID_SIZE;

			game->HandleClick(pixelX, pixelY, cellWidth, cellHeight);
		}
		break;
	}

	case WM_SETFOCUS:
		EnableWindow(hWnd, TRUE);
		break;

	case WM_DESTROY:
		KillTimer(hWnd, 1);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void PreloadShell() {
	LoadLibraryW(L"shell32.dll");
	LoadLibraryW(L"comctl32.dll");
	LoadLibraryW(L"shlwapi.dll");
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	LoadSettingsFromRegistry();
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (FAILED(hr)) {
		MessageBoxW(NULL, L"com", L"err", MB_ICONERROR);
		return 0;
	}

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::Status status = Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	if (status != Gdiplus::Ok) {
		MessageBoxW(NULL, L"FailedGDI+", L"Error", MB_ICONERROR);
		CoUninitialize();
		return 0;
	}

	std::thread([]() {
		PreloadShell();
		}).detach();

	WNDCLASSEX wc = {
	sizeof(WNDCLASSEX),
	CS_HREDRAW | CS_VREDRAW,
	WndProc,
	0, 0, hInstance, NULL, NULL, NULL, NULL,
	L"GemClass", NULL
	};
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClassEx(&wc);

	WNDCLASSEX wcSettings = {
	sizeof(WNDCLASSEX),
	CS_HREDRAW | CS_VREDRAW,
	SettingsWndProc,
	0, 0, hInstance, NULL, NULL, NULL, NULL,
	L"SettingsClass", NULL
	};
	wcSettings.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClassEx(&wcSettings);

	int windowWidth = 416;
	int windowHeight = 439;
	{
		auto game = std::make_unique<Game>();

		HWND hWnd = CreateWindowEx(
			0,
			L"GemClass",
			L"tri v ryad",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			windowWidth, windowHeight,
			NULL, NULL, hInstance, game.get()
		);

		HMENU hMenu = CreateMenu();
		HMENU hSubMenu = CreatePopupMenu();
		AppendMenuW(hSubMenu, MF_STRING, ID_MENU_RESTART, L"Рестарт");
		AppendMenuW(hSubMenu, MF_STRING, ID_MENU_SETTINGS, L"Настройки...");
		AppendMenuW(hSubMenu, MF_SEPARATOR, 0, NULL);
		AppendMenuW(hSubMenu, MF_STRING, ID_MENU_EXIT, L"Выход");
		AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"Игра");
		SetMenu(hWnd, hMenu);

		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);

		GameObserver observer(game.get());
		game->AddObserver(&observer);

		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		game->RemoveObserver(&observer);
	} 

	Gdiplus::GdiplusShutdown(gdiplusToken);
	CoUninitialize();

	return 0;
}