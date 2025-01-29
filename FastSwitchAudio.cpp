// FastSwitchAudio.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "FastSwitchAudio.h"
#include <cstdint>
#include <format>
#include <vector>
#include <mmdeviceapi.h>
#include <atlbase.h>
#include <CommCtrl.h>
#include <uxtheme.h>
#include "AboutDialog.h"
#pragma comment(lib, "comctl32")
#pragma comment(lib, "uxtheme")

/**
 * @brief Maximum size we're allocating for strings when using `LoadStringW`.
 */
constexpr std::uint32_t maxLoadString = 128;

HINSTANCE hInst;
HWND mainWindow;
HWND listView;

CComPtr<IMMDeviceEnumerator> mmDeviceEnumerator;
CComPtr<IMMDeviceCollection> audioOutputs;

WCHAR windowTitle[maxLoadString];                  // The title bar text
WCHAR windowClassName[maxLoadString];            // the main window class name

void loadStrings(HINSTANCE hInstance);
ATOM                registerWindowClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE __prevInst,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       showWindowMode
) {

	hInst = hInstance;

	if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) {
		return FALSE;
	}

	const INITCOMMONCONTROLSEX commonControlsInitData {
		.dwSize = sizeof(INITCOMMONCONTROLSEX),
		.dwICC = ICC_LISTVIEW_CLASSES
	};

	InitCommonControlsEx(&commonControlsInitData);
	
	loadStrings(hInst);
	registerWindowClass(hInst);
	
	mainWindow = CreateWindow(
		windowClassName,
		windowTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInst, nullptr
	);

	if (mainWindow == nullptr) {
		return FALSE;
	}

	ShowWindow(mainWindow, showWindowMode);
	UpdateWindow(mainWindow);

	if (FAILED(mmDeviceEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator)))) {
		MessageBox(mainWindow, L"Failed to get an audio device enumerator instance!", L"Fatal Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	
	if (FAILED(mmDeviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &audioOutputs))) {
		MessageBox(mainWindow, L"Failed to retrieve list of audio output devices", L"Fatal Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	
	std::uint32_t outputCount;

	if (FAILED(audioOutputs->GetCount(&outputCount))) {
		MessageBox(mainWindow, L"Failed to count # of audio output devices.", L"Fatal Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	RECT clientRect;
	GetClientRect(mainWindow, &clientRect);

	listView = CreateWindow(WC_LISTVIEW, L"",
		WS_CHILD | LVS_LIST | LVS_SINGLESEL,
		0, 0,
		clientRect.right - clientRect.left,
		clientRect.bottom - clientRect.top,
		mainWindow,
		nullptr, hInst, nullptr
	);

	//SetWindowTheme(listView, L"Explorer", nullptr);

	for (std::uint32_t i = 0; i < outputCount; i ++) {
		
		CComPtr<IMMDevice> device;
		LPWSTR id;
		
		if (FAILED(audioOutputs->Item(i, &device))) {
			// TODO!
			return FALSE;
		}

		if (FAILED(device->GetId(&id))) {
			// TODO!
			return FALSE;
		}

		const LVITEM listItem {
			.mask = LVIF_TEXT,
			.iItem = static_cast<int>(i),
			.pszText = id,
		};

		if (ListView_InsertItem(listView, &listItem) == -1) {
			return FALSE;
		}

		CoTaskMemFree(id);

	}

	ShowWindow(listView, showWindowMode);
	UpdateWindow(listView);

	HACCEL acceleratorTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_FASTSWITCHAUDIO));
	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, acceleratorTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

void loadStrings(HINSTANCE hInstance) {
	LoadString(hInst, IDS_APP_TITLE, windowTitle, maxLoadString);
	LoadString(hInst, IDC_FASTSWITCHAUDIO, windowClassName, maxLoadString);
}

/**
 * @brief Registers our window's class.
 * @param hInstance
 * @returns Unique identifier of the class.
 */
ATOM registerWindowClass(HINSTANCE hInstance) {

	WNDCLASSEXW windowClass {
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = WndProc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = hInstance,
		.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FASTSWITCHAUDIO)),
		.hCursor = LoadCursor(nullptr, IDC_ARROW),
		.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME),
		.lpszMenuName = MAKEINTRESOURCEW(IDC_FASTSWITCHAUDIO),
		.lpszClassName = windowClassName,
		.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL)),
	};

	return RegisterClassEx(&windowClass);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {

	switch (message) {

		case WM_COMMAND: {

			int wmId = LOWORD(wParam);

			// Parse the menu selections:
			switch (wmId) {

				case IDM_ABOUT: {
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), window, About);
					break;
				}

				case IDM_EXIT: {
					DestroyWindow(window);
					break;
				}

				default: {
					return DefWindowProc(window, message, wParam, lParam);
				}

			}
			
			break;
		}

		case WM_NOTIFY: {

			NMHDR* info = reinterpret_cast<NMHDR*>(lParam);
			
			if (info->hwndFrom == listView) {
				switch (info->code) {

					case NM_DBLCLK: {
					
						std::int32_t itemIndex = ListView_GetNextItem(listView, -1, LVNI_SELECTED);

						if (itemIndex < 0) {
							break;
						}

						wchar_t buffer[64];
						
						LVITEM item {
							.mask = LVIF_TEXT,
							.iItem = itemIndex,
							.pszText = buffer,
							.cchTextMax = sizeof(buffer) / sizeof(wchar_t)
						};

						ListView_GetItem(listView, &item);

						MessageBox(mainWindow, buffer, L"Chosen item", MB_OK | MB_ICONINFORMATION);
						
						break;
					}

				}
			}

			break;

		}

		case WM_SIZE: {
			
			RECT clientRect;
			GetClientRect(mainWindow, &clientRect);

			SetWindowPos(listView, nullptr,
				0, 0,
				clientRect.right - clientRect.left,
				clientRect.bottom - clientRect.top,
				SWP_NOACTIVATE
			);

			break;

		}
		
		case WM_PAINT: {
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(window, &ps);
				
				//FillRect(hdc, &ps.rcPaint, HBRUSH(COLOR_BACKGROUND + 2));

				EndPaint(window, &ps);
			break;
		}

		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}

		default: {
			return DefWindowProc(window, message, wParam, lParam);
		}

	}

	return 0;
}
