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

struct MenuItemData
{

	enum class Type
	{
		AudioDevice,
		ExitButton,
	};

	/**
	 * @brief The type of item this is.
	 */
	const Type type;

	/**
	 * @brief Associated audio device ID for this menu item, if it represents one.
	 */
	const std::optional<Audio::Device::Id> audioDeviceId;

	static MenuItemData device(Audio::Device::Id deviceId)
	{
		return MenuItemData{
			.type = Type::AudioDevice,
			.audioDeviceId = deviceId
		};
	}

	static MenuItemData exitButton()
	{
		return MenuItemData{
			.type = Type::ExitButton,
			.audioDeviceId = std::nullopt
		};
	}

};

static Audio::DeviceManager audioDeviceManager;
static std::vector<MenuItemData> popupMenuItems = { MenuItemData{} };
static HMENU popupMenu;

HWND trayWindow;
WCHAR windowClassName[maxLoadString];

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

/**
 * @brief Show a fatal error message box if something went wrong we cannot recover from.
 * @param message The message to be shown.
 */
static void showFatalError(const std::wstring message);

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE __prevInst,
	_In_ LPWSTR lpCmdLine,
	_In_ int showWindowMode
)
{
	
	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		showFatalError(L"COM failed to initialise, which we need for querying devices!");
		return FALSE;
	}

	popupMenu = CreatePopupMenu();

	if (popupMenu == NULL)
	{
		showFatalError(L"Failed to create a popup context menu!");
		return FALSE;
	}

	LoadStringW(hInstance, IDC_FASTSWITCHAUDIO, windowClassName, maxLoadString);

	if (const auto maybeError = audioDeviceManager.refresh(); maybeError.has_value())
	{
		Audio::Error error = maybeError.value();
		showFatalError(error.explanation);

		return FALSE;
	}

	for (const Audio::Device& device : audioDeviceManager.devices)
	{
		const UINT itemId = popupMenuItems.size();
		std::wstring itemLabel = std::wstring(device.getName());

		MENUITEMINFOW item{
			.cbSize = sizeof(MENUITEMINFOW),
			.fMask = MIIM_STRING | MIIM_STATE | MIIM_ID,
			.fState = MFS_ENABLED,
			.wID = itemId,
			.dwTypeData = itemLabel.data()
		};

		popupMenuItems.emplace_back(MenuItemData::device(device.id));
		InsertMenuItemW(popupMenu, itemId, TRUE, &item);
	}

	{
		const UINT itemId = popupMenuItems.size();
		std::wstring label = L"Quit";

		MENUITEMINFOW item{
			.cbSize = sizeof(MENUITEMINFOW),
			.fMask = MIIM_STRING | MIIM_STATE | MIIM_ID,
			.fState = MFS_ENABLED,
			.wID = itemId,
			.dwTypeData = label.data()
		};

		popupMenuItems.emplace_back(MenuItemData::exitButton());
		InsertMenuItemW(popupMenu, itemId, TRUE, &item);
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
		showFatalError(L"Failed to create the tray window!");
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
	
	if (!Shell_NotifyIcon(NIM_ADD, &trayIconData))
	{
		showFatalError(L"Failed to add a tray icon to the taskbar!");
		return FALSE;
	}

	if (!Shell_NotifyIcon(NIM_SETVERSION, &trayIconData))
	{
		showFatalError(L"Apparently running on Windows earlier than Vista! This is not supported right now.");
		return FALSE;
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
		case static_cast<UINT>(Notification::ToggleTrayIcon):
		{
			switch (LOWORD(lParam))
			{
				case WM_RBUTTONDOWN:
				{
					POINT cursorPos;
					GetCursorPos(&cursorPos);

					SetForegroundWindow(trayWindow);

					constexpr UINT popupFlags = TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD;
					const UINT itemId = TrackPopupMenu(popupMenu, popupFlags, cursorPos.x, cursorPos.y, 0, trayWindow, NULL);

					if (itemId == NULL)
					{
						break;
					}

					const MenuItemData itemData = popupMenuItems[itemId];

					switch (itemData.type)
					{
						case MenuItemData::Type::AudioDevice:
						{
							Audio::Device::Id deviceId = itemData.audioDeviceId.value();
							Audio::Device& device = audioDeviceManager[deviceId];

							device.setAsDefault();
							break;
						}

						case MenuItemData::Type::ExitButton:
						{
							PostQuitMessage(0);
							break;
						}
					}

					break;
				}

				default:
				{
					return DefWindowProcW(window, message, wParam, lParam);
				}
			}

			break;
		}

		default:
		{
			return DefWindowProcW(window, message, wParam, lParam);
		}

	}

	return 0;
}

void showFatalError(const std::wstring message)
{
	MessageBoxW(NULL, message.c_str(), L"Fatal Error", MB_OK | MB_ICONERROR);
}
