// FastSwitchAudio.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "FastSwitchAudio.h"
#include <cstdint>
#include <format>
#include <mmdeviceapi.h>
#include "AboutDialog.h"

/**
 * @brief Maximum size we're allocating for strings when using `LoadStringW`.
 */
constexpr std::uint32_t maxLoadString = 128;

HINSTANCE hInst;
HWND mainWindow;

IMMDeviceEnumerator* mmDeviceEnumerator;
IMMDeviceCollection* audioOutputs;

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

	if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) {
		return FALSE;
	}

	hInst = hInstance;
	
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

	if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&mmDeviceEnumerator)))) {
		MessageBox(mainWindow, L"Failed to get an audio device enumerator instance!", L"Fatal Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	
	if (FAILED(mmDeviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &audioOutputs))) {
		
		MessageBox(mainWindow, L"Failed to retrieve list of audio output devices", L"Fatal Error", MB_OK | MB_ICONERROR);
		mmDeviceEnumerator->Release();
		
		return FALSE;
	}
	
	std::uint32_t outputCount;

	if (FAILED(audioOutputs->GetCount(&outputCount))) {
		
		MessageBox(mainWindow, L"Failed to count # of audio output devices.", L"Fatal Error", MB_OK | MB_ICONERROR);
		audioOutputs->Release();
		mmDeviceEnumerator->Release();

		return FALSE;
	}

	MessageBox(mainWindow, std::format(L"Found {} enabled audio ouputs!", outputCount).c_str(), L"Hello!", MB_OK | MB_ICONINFORMATION);

	for (std::uint32_t i = 0; i < outputCount; i ++) {
		
		IMMDevice* output;
		LPWSTR id;
		
		if (FAILED(audioOutputs->Item(i, &output))) {
			// TODO!
		}

		if (FAILED(output->GetId(&id))) {
			// TODO!
		}

		MessageBox(mainWindow, id, L"Audio device!", MB_OK | MB_ICONINFORMATION);
		output->Release();

	}
	

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
		
		case WM_PAINT: {
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(window, &ps);
				// TODO: Add any drawing code that uses hdc here...
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
