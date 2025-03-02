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

/**
 * @brief Maximum size we're allocating for strings when using `LoadStringW`.
 */
constexpr UINT maxLoadString = 128;

enum class Notification : UINT
{
	ToggleTrayIcon = (WM_USER + 0x100)
};

/**
 * @brief Show a fatal error message box if something went wrong we cannot recover from.
 * @param message The message to be shown.
 */
static void ShowFatalError(const std::wstring message)
{
	MessageBoxW(NULL, message.c_str(), L"Fatal Error", MB_OK | MB_ICONERROR);
}

/**
 * @brief Main window procedure.
 */
LRESULT CALLBACK TrayWindowProc(HWND, UINT, WPARAM, LPARAM);
