#pragma once

#include "resource.h"
#include "framework.h"
#include <optional>
#include <cstdint>
#include <format>
#include <vector>
#include <ranges>
#include <windowsx.h>
#include <shellapi.h>
#include "Audio/Audio.h"
#include "Audio/DeviceManager.h"
#include "PopupMenu.h"
#include "utils.h"

#pragma comment(lib, "comctl32")
#pragma comment(lib, "uxtheme")

#pragma comment(linker, "\"/manifestdependency:type='win32'	\
	name='Microsoft.Windows.Common-Controls'				\
	version='6.0.0.0'										\
	processorArchitecture='*'								\
	publicKeyToken='6595b64144ccf1df'						\
	language='*'											\
\"")

struct MenuItemData
{

	enum class Type
	{
		AudioDevice,
		RefreshButton,
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

	static MenuItemData button(Type type)
	{
		return MenuItemData{
			.type = type,
			.audioDeviceId = std::nullopt
		};
	}

};

enum class Notification : UINT
{
	ToggleTrayIcon = (WM_USER + 0x100)
};

/**
 * @brief Maximum size we're allocating for strings when using `LoadStringW`.
 */
constexpr UINT maxLoadString = 128;

/**
 * @brief Identifer for the tray icon we create.
 */
constexpr UINT trayIconId = 1;

static WCHAR g_trayWindowClassName[maxLoadString];
static HWND g_trayWindow;

static Audio::DeviceManager* g_audioDeviceManager;
static UI::PopupMenu<MenuItemData>* g_popupMenu;

/**
 * @brief Refresh the contents of the popup menu for changes in the device list.
 * Ideally, we'd only change what needs changing, but this is the lazy way out for now.
 */
static void RefreshPopupMenu();

/**
 * @brief Main window procedure.
 */
LRESULT CALLBACK TrayWindowProc(HWND, UINT, WPARAM, LPARAM);
