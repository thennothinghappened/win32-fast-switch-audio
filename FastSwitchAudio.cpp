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
#include "Audio/DeviceManagerImpl.h"

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
constexpr std::uint32_t maxLoadString = 128;

enum class Notification : std::uint32_t
{
	ToggleTrayIcon = (WM_USER + 0x100)
};

Audio::DeviceManagerImpl audioDeviceManager;

HINSTANCE hInst;
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
	hInst = hInstance;

	if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		return FALSE;
	}

	LoadStringW(hInst, IDC_FASTSWITCHAUDIO, windowClassName, maxLoadString);

	if (auto maybeError = audioDeviceManager.refresh(); maybeError.has_value())
	{
		Audio::Error error = maybeError.value();
		MessageBoxW(nullptr, error.explanation.c_str(), L"Fatal Error", MB_OK | MB_ICONERROR);

		return FALSE;
	}

	const WNDCLASSEXW windowClass
	{
		.cbSize = sizeof(WNDCLASSEX),
		.lpfnWndProc = WndProc,
		.hInstance = hInst,
		.hIcon = LoadIconW(hInst, MAKEINTRESOURCE(IDI_FASTSWITCHAUDIO)),
		.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME),
		.lpszMenuName = MAKEINTRESOURCEW(IDC_FASTSWITCHAUDIO),
		.lpszClassName = windowClassName,
		.hIconSm = LoadIconW(hInst, MAKEINTRESOURCE(IDI_SMALL)),
	};

	RegisterClassExW(&windowClass);

	trayWindow = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		windowClassName, L"", WS_POPUP,
		0, 0, 0, 0, nullptr, nullptr, hInst, nullptr
	);

	if (trayWindow == nullptr)
	{
		return FALSE;
	}

	NOTIFYICONDATA trayIconData{
		.cbSize = sizeof(NOTIFYICONDATA),
		.hWnd = trayWindow,
		.uID = 1,
		.uFlags = NIF_MESSAGE | NIF_ICON,
		.uCallbackMessage = static_cast<std::uint32_t>(Notification::ToggleTrayIcon),
		.hIcon = LoadIconW(hInst, MAKEINTRESOURCE(IDI_SMALL)),
		.uVersion = NOTIFYICON_VERSION_4
	};
	
	if (Shell_NotifyIcon(NIM_ADD, &trayIconData))
	{
		Shell_NotifyIcon(NIM_SETVERSION, &trayIconData);
	}

	HACCEL acceleratorTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_FASTSWITCHAUDIO));
	MSG msg;

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



LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HMENU menu = CreatePopupMenu();

	switch (message)
	{

		case WM_CREATE:
		{
			for (std::uint32_t i = 0; i < audioDeviceManager.count(); i++)
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

				InsertMenuItemW(menu, i, TRUE, &item);
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

					constexpr std::uint32_t popupFlags = TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD;
					const std::uint32_t itemId = TrackPopupMenu(menu, popupFlags, cursorPos.x, cursorPos.y, 0, trayWindow, nullptr);

					if (itemId & 0b1000000)
					{
						const std::uint32_t deviceId = itemId ^ 0b1000000;
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
