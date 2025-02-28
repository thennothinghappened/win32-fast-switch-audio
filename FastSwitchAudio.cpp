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
#include <cassert>
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

enum UserMessage
{
	WM_ToggleTrayIcon = (WM_USER + 1)
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/**
 * @brief Maximum size we're allocating for strings when using `LoadStringW`.
 */
constexpr uint32_t maxLoadString = 128;

static Audio::DeviceManagerImpl audioDeviceManager;

static HWND trayWindow;
static WCHAR windowClassName[maxLoadString];

static HMENU popupMenu = CreatePopupMenu();

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
		.uCallbackMessage = WM_ToggleTrayIcon,
		.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_SMALL)),
		.uVersion = NOTIFYICON_VERSION_4
	};
	
	if (Shell_NotifyIconW(NIM_ADD, &trayIconData) == false)
	{
		return FALSE;
	}

	Shell_NotifyIconW(NIM_SETVERSION, &trayIconData);

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

/**
 * @brief Identifying data for a given menu item in the popup menu.
 */
using MenuItem = struct
{

	enum class Type : UCHAR
	{
		AudioDevice,
		Separator,
		ExitButton,
	};

	/**
	 * @brief The type of item this is.
	 */
	const Type type;

	/**
	 * @brief Unique
	 */
	const UCHAR id;

	/**
	 * @brief Data associated with this item.
	 */
	const union
	{
		/**
		 * @brief ID of the audio device this item is representing.
		 */
		USHORT AudioDevice_id;

		/**
		 * @brief Empty padding data for self-explanatory item types.
		 */
		USHORT filler;
	} data;

	/**
	 * @brief Pack this item into the form Windows provides for storing menu item data.
	 * @return An unsigned integer that contains the same data this item is storing.
	 */
	const UINT pack() const
	{
		UINT packed;
		std::memcpy(&packed, this, sizeof(UINT));

		return packed;
	}

	/**
	 * @brief Unpack this item from the form provided by Windows.
	 * @param packed The packed data to be reinterpreted as a menu item.
	 * @return The represented item.
	 */
	static const MenuItem unpack(UINT packed)
	{
		MenuItem item{};
		std::memcpy(&packed, &item, sizeof(UINT));

		return item;
	}

	static_assert(sizeof(MenuItem) == sizeof(UINT), "Menu items must be storable within the provided `UINT` we're given to ID them with.");
};


static MENUITEMINFOW createMenuSeparator(UINT id)
{
	MenuItem item{
		.type = MenuItem::Type::Separator
	};

	return MENUITEMINFOW{
		.cbSize = sizeof(MENUITEMINFOW),
		.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA,
		.fType = MFT_SEPARATOR,
		.dwItemData = item.pack()
	};
}

static void initPopupMenu()
{
	UINT menuIndex = 1;

	for (UINT deviceIndex = 0; deviceIndex < audioDeviceManager.count(); deviceIndex++)
	{
		const Audio::Device& device = audioDeviceManager[deviceIndex];
		std::wstring itemLabel = std::wstring(device.getName());

		MENUITEMINFOW item{
			.cbSize = sizeof(MENUITEMINFOW),
			.fMask = MIIM_STRING | MIIM_STATE | MIIM_ID | MIIM_DATA,
			.fState = MFS_ENABLED,
			.wID = menuIndex,
			.dwItemData = MenuItem{
				.type = MenuItem::Type::AudioDevice,
				.data = static_cast<USHORT>(deviceIndex)
			}.pack(),
			.dwTypeData = itemLabel.data()
		};

		InsertMenuItemW(popupMenu, menuIndex, TRUE, &item);
		menuIndex++;
	}


	MENUITEMINFOW separatorItem = createMenuSeparator(menuIndex);
	InsertMenuItemW(popupMenu, menuIndex, TRUE, &separatorItem);
	menuIndex++;

	MENUITEMINFOW exitItem{
		.cbSize = sizeof(MENUITEMINFOW),
		.fMask = MIIM_STRING | MIIM_STATE | MIIM_ID | MIIM_DATA,
		.fState = MFS_ENABLED,
		.wID = menuIndex,
		.dwItemData = MenuItem{
			.type = MenuItem::Type::ExitButton
		}.pack(),
		.dwTypeData = std::wstring(L"Exit").data()
	};

	InsertMenuItemW(popupMenu, menuIndex, TRUE, &exitItem);
	menuIndex++;
}

static void showPopupMenu()
{
	SetForegroundWindow(trayWindow);

	POINT cursor;
	GetCursorPos(&cursor);

	constexpr UINT popupFlags = TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD;
	UINT itemId = TrackPopupMenu(popupMenu, popupFlags, cursor.x, cursor.y, 0, trayWindow, NULL);

	if (itemId == 0)
	{
		return;
	}

	if (itemId & 0b1000000)
	{
		uint32_t deviceId = itemId ^ 0b1000000;
		audioDeviceManager[deviceId].setAsDefault();
	}
}

LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			initPopupMenu();
			break;
		}

		case WM_ToggleTrayIcon:
		{
			switch (LOWORD(lParam))
			{
				case WM_RBUTTONDOWN:
				{
					showPopupMenu();
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
