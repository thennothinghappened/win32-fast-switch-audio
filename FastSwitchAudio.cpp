// FastSwitchAudio.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "FastSwitchAudio.h"
#include <cstdint>
#include <format>
#include <vector>
#include <ranges>
#include "AboutDialog.h"
#include "Audio/Audio.h"
#include "Audio/DeviceManagerImpl.h"
#include "ListView.h"
#pragma comment(lib, "comctl32")
#pragma comment(lib, "uxtheme")

/**
 * @brief Maximum size we're allocating for strings when using `LoadStringW`.
 */
constexpr std::uint32_t maxLoadString = 128;

Audio::DeviceManagerImpl audioDeviceManager;

HINSTANCE hInst;
HWND mainWindow;

WCHAR windowClassName[maxLoadString];
WCHAR windowTitle[maxLoadString];                  // The title bar text

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE __prevInst,
	_In_ LPWSTR lpCmdLine,
	_In_ int showWindowMode
)
{
	hInst = hInstance;

	if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		return FALSE;
	}

	LoadStringW(hInst, IDC_FASTSWITCHAUDIO, windowClassName, maxLoadString);
	LoadStringW(hInst, IDS_APP_TITLE, windowTitle, maxLoadString);

	if (auto maybeError = audioDeviceManager.refresh(); maybeError.has_value())
	{
		Audio::Error error = maybeError.value();
		MessageBoxW(nullptr, error.explanation.c_str(), L"Fatal Error", MB_OK | MB_ICONERROR);

		return FALSE;
	}

	const WNDCLASSEXW windowClass
	{
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

	RegisterClassExW(&windowClass);

	mainWindow = CreateWindowW(
		windowClassName,
		windowTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInst, nullptr
	);

	if (mainWindow == nullptr)
	{
		return FALSE;
	}

	ShowWindow(mainWindow, showWindowMode);
	UpdateWindow(mainWindow);

	HACCEL acceleratorTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_FASTSWITCHAUDIO));
	MSG msg;

	// Main message loop:
	while (GetMessageW(&msg, nullptr, 0, 0))
	{
		if (!TranslateAcceleratorW(msg.hwnd, acceleratorTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	return (int)msg.wParam;
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
LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	static ListView* listView;

	switch (message)
	{

		case WM_CREATE:
		{
			listView = new ListView(window);
			listView->updateSize();

			for (std::size_t i = 0; i < audioDeviceManager.count(); i++)
			{
				const Audio::Device& device = audioDeviceManager[i];
				listView->insert(i, device.getName());
			}

			break;
		}

		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);

			// Parse the menu selections:
			switch (wmId)
			{

				case IDM_ABOUT:
				{
					DialogBoxW(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), window, About);
					break;
				}

				case IDM_EXIT:
				{
					DestroyWindow(window);
					break;
				}

				default:
				{
					return DefWindowProcW(window, message, wParam, lParam);
				}

			}
			break;
		}

		case WM_NOTIFY:
		{
			const NMHDR* notification = reinterpret_cast<NMHDR*>(lParam);
			
			if (auto maybeItemIndex = listView->handleNotification(notification); maybeItemIndex.has_value())
			{
				std::int32_t itemIndex = maybeItemIndex.value();
				audioDeviceManager[itemIndex].setAsDefault();

				break;
			}
			
			break;
		}

		case WM_SIZE:
		{
			listView->updateSize();
			break;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(window, &ps);

			EndPaint(window, &ps);

			break;
		}

		case WM_DESTROY:
		{
			delete listView;
			PostQuitMessage(0);

			break;
		}

		default:
		{
			return DefWindowProcW(window, message, wParam, lParam);
		}

	}

	return 0;
}
