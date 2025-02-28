// FastSwitchAudio.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "FastSwitchAudio.h"
#include <cstdint>
#include <format>
#include <vector>
#include <ranges>
#include <windowsx.h>
#include <shellapi.h>
#include "Audio/Audio.h"
#include "Audio/DeviceManager.h"

#pragma comment(lib, "comctl32")
#pragma comment(lib, "uxtheme")

#pragma comment(linker, "\"/manifestdependency:type='win32'	\
	name='Microsoft.Windows.Common-Controls'				\
	version='6.0.0.0'										\
	processorArchitecture='*'								\
	publicKeyToken='6595b64144ccf1df'						\
	language='*'											\
\"")

/**
 * @brief Maximum size we're allocating for strings when using `LoadStringW`.
 */
constexpr uint32_t maxLoadString = 128;

enum class Notification : uint32_t
{
	ToggleTrayIcon = (WM_USER + 0x100)
};

Audio::DeviceManager audioDeviceManager;


static HMENU popupMenu;
HWND trayWindow;
WCHAR windowClassName[maxLoadString];

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE __prevInst,
	_In_ LPWSTR lpCmdLine,
	_In_ int showWindowMode
)
{

	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		return FALSE;
	}

	popupMenu = CreatePopupMenu();

	if (popupMenu == NULL)
	{
		return FALSE;
	}

	LoadStringW(hInstance, IDC_FASTSWITCHAUDIO, windowClassName, maxLoadString);

	if (auto maybeError = audioDeviceManager.refresh(); maybeError.has_value())
	{
		Audio::Error error = maybeError.value();
		MessageBoxW(NULL, error.explanation.c_str(), L"Fatal Error", MB_OK | MB_ICONERROR);

		return FALSE;
	}

	const WNDCLASSEXW windowClass
	{
		.cbSize = sizeof(WNDCLASSEX),
		.lpfnWndProc = WndProc,
		.hInstance = hInstance,
		.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_FASTSWITCHAUDIO)),
		.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME),
		.lpszMenuName = MAKEINTRESOURCEW(IDC_FASTSWITCHAUDIO),
		.lpszClassName = windowClassName,
		.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_SMALL)),
	};

	RegisterClassExW(&windowClass);

	trayWindow = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		windowClassName, L"", WS_POPUP,
		0, 0, 0, 0, NULL, NULL, hInstance, NULL
	);

	if (trayWindow == NULL)
	{
		return FALSE;
	}

	NOTIFYICONDATA trayIconData{
		.cbSize = sizeof(NOTIFYICONDATA),
		.hWnd = trayWindow,
		.uID = 1,
		.uFlags = NIF_MESSAGE | NIF_ICON,
		.uCallbackMessage = static_cast<UINT>(Notification::ToggleTrayIcon),
		.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_SMALL)),
		.uVersion = NOTIFYICON_VERSION_4
	};
	
	if (Shell_NotifyIcon(NIM_ADD, &trayIconData))
	{
		Shell_NotifyIcon(NIM_SETVERSION, &trayIconData);
	}

	HACCEL acceleratorTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FASTSWITCHAUDIO));
	MSG msg;

	while (GetMessageW(&msg, NULL, 0, 0))
	{
		if (!TranslateAcceleratorW(msg.hwnd, acceleratorTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	return (int)msg.wParam;
}



LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

		case WM_CREATE:
		{
			for (uint32_t i = 0; i < audioDeviceManager.count(); i++)
			{
				const Audio::Device& device = audioDeviceManager[i];
				std::wstring itemLabel = std::wstring(device.getName());

				MENUITEMINFOW item{
					.cbSize = sizeof(MENUITEMINFOW),
					.fMask = MIIM_STRING | MIIM_STATE | MIIM_ID,
					.fState = MFS_ENABLED,
					.wID = i | 0b1000000, // FIXME: evil magic!!
					.dwTypeData = itemLabel.data()
				};

				InsertMenuItemW(popupMenu, i, TRUE, &item);
			}

			break;
		}

		case static_cast<UINT>(Notification::ToggleTrayIcon):
		{
			switch (LOWORD(lParam))
			{
				case WM_RBUTTONDOWN:
				{
					POINT cursorPos;
					GetCursorPos(&cursorPos);

					SetForegroundWindow(trayWindow);

					constexpr uint32_t popupFlags = TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD;
					const uint32_t itemId = TrackPopupMenu(popupMenu, popupFlags, cursorPos.x, cursorPos.y, 0, trayWindow, NULL);

					if (itemId & 0b1000000)
					{
						const uint32_t deviceId = itemId ^ 0b1000000;
						audioDeviceManager[deviceId].setAsDefault();

						break;
					}

					break;
				}
			}

			return DefWindowProcW(window, message, wParam, lParam);
		}

		default:
		{
			return DefWindowProcW(window, message, wParam, lParam);
		}

	}

	return 0;
}
