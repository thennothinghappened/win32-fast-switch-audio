// FastSwitchAudio.cpp : Defines the entry point for the application.
//

#include "FastSwitchAudio.h"
#include <cstdint>
#include <ranges>
#include <vector>
#include "resource.h"
#include "utils.h"

int APIENTRY wWinMain(
	_In_ const HINSTANCE hInstance,
	_In_opt_ HINSTANCE,
	_In_ LPWSTR,
	_In_ int
)
{
	if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		ShowFatalError(L"COM failed to initialise, which we need for querying devices!");
		return FALSE;
	}

	LoadStringW(hInstance, IDC_FASTSWITCHAUDIO, g_trayWindowClassName, maxLoadString);

	const WNDCLASSEXW windowClass
	{
		.cbSize = sizeof(WNDCLASSEXW),
		.lpfnWndProc = TrayWindowProc,
		.hInstance = hInstance,
		.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_FASTSWITCHAUDIO)),
		.lpszMenuName = MAKEINTRESOURCEW(IDC_FASTSWITCHAUDIO),
		.lpszClassName = g_trayWindowClassName,
		.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_SMALL)),
	};

	RegisterClassExW(&windowClass);

	g_trayWindow = CreateWindowExW(
		WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		g_trayWindowClassName,
		L"",
		WS_POPUP,
		0,
		0,
		0,
		0,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	if (g_trayWindow == nullptr)
	{
		ShowFatalError(L"Failed to create the tray window!");
		return FALSE;
	}

	const HMENU popupMenuHandle = CreatePopupMenu();

	if (popupMenuHandle == nullptr)
	{
		ShowFatalError(L"Failed to create a popup context menu!");
		return FALSE;
	}

	g_popupMenu = new UI::PopupMenu<MenuItemData>(g_trayWindow, popupMenuHandle);
	g_audioDeviceManager = new Audio::DeviceManager(RefreshPopupMenu, ShowFatalError);

	if (const auto error = g_audioDeviceManager->refresh())
	{
		ShowFatalError(error->explanation);
		return FALSE;
	}

	NOTIFYICONDATAW trayIconData {
		.cbSize = sizeof(NOTIFYICONDATAW),
		.hWnd = g_trayWindow,
		.uID = trayIconId,
		.uFlags = NIF_MESSAGE | NIF_ICON,
		.uCallbackMessage = static_cast<UINT>(Notification::ToggleTrayIcon),
		.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_SMALL)),
		.uVersion = NOTIFYICON_VERSION_4
	};

	if (!Shell_NotifyIconW(NIM_ADD, &trayIconData))
	{
		ShowFatalError(L"Failed to add a tray icon to the taskbar!");
		return FALSE;
	}

	if (!Shell_NotifyIconW(NIM_SETVERSION, &trayIconData))
	{
		ShowFatalError(L"Apparently running on Windows earlier than Vista! This is not supported right now.");
		return FALSE;
	}

	const auto acceleratorTable = LoadAcceleratorsW(hInstance, MAKEINTRESOURCE(IDC_FASTSWITCHAUDIO));
	MSG msg;

	while (GetMessageW(&msg, nullptr, 0, 0))
	{
		if (!TranslateAcceleratorW(msg.hwnd, acceleratorTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	return static_cast<int>(msg.wParam);
}

void RefreshPopupMenu()
{
	g_popupMenu->clear();

	const Audio::Device& defaultConsoleDevice = g_audioDeviceManager->getDefault(eConsole);
	const Audio::Device& defaultMediaDevice = g_audioDeviceManager->getDefault(eMultimedia);
	const Audio::Device& defaultCommsDevice = g_audioDeviceManager->getDefault(eCommunications);

	for (const Audio::Device& device : g_audioDeviceManager->devices)
	{
		const bool isDefault = (device.id == defaultConsoleDevice.id)
		                       || (device.id == defaultMediaDevice.id)
		                       || (device.id == defaultCommsDevice.id);

		const auto label = device.getName();

		// TODO: C++ may be sneakily doing the whole copy constructor business here, I haven't checked though.
		g_popupMenu->append(MenuItemData::device(device.id), label, isDefault);
	}

	g_popupMenu->appendSeparator();
	g_popupMenu->append(MenuItemData::button(MenuItemData::Type::RefreshButton), L"Refresh");
	g_popupMenu->append(MenuItemData::button(MenuItemData::Type::ExitButton), L"Quit");
}

LRESULT CALLBACK TrayWindowProc(
	const HWND window,
	const UINT message,
	const WPARAM wParam,
	const LPARAM lParam
)
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

					SetForegroundWindow(g_trayWindow);
					std::optional<MenuItemData> maybeItem = g_popupMenu->track(cursorPos);

					if (!maybeItem.has_value())
					{
						break;
					}

					const auto item = maybeItem.value();

					switch (item.type)
					{
						case MenuItemData::Type::AudioDevice:
						{
							Audio::Device::Id deviceId = item.audioDeviceId.value();
							g_audioDeviceManager->setDefault(deviceId);

							break;
						}

						case MenuItemData::Type::RefreshButton:
						{
							if (const auto error = g_audioDeviceManager->refresh())
							{
								ShowFatalError(error->explanation);
								break;
							}

							break;
						}

						case MenuItemData::Type::ExitButton:
						{
							DestroyWindow(g_trayWindow);
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

		case WM_DESTROY:
		{
			NOTIFYICONDATAW trayIconData {
				.cbSize = sizeof(NOTIFYICONDATAW),
				.hWnd = g_trayWindow,
				.uID = trayIconId
			};

			Shell_NotifyIconW(NIM_DELETE, &trayIconData);

			delete g_audioDeviceManager;
			delete g_popupMenu;

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
