// FastSwitchAudio.cpp : Defines the entry point for the application.
//

#include "FastSwitchAudio.h"

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE __prevInst,
	_In_ LPWSTR lpCmdLine,
	_In_ int showWindowMode
)
{
	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		ShowFatalError(L"COM failed to initialise, which we need for querying devices!");
		return FALSE;
	}
	
	LoadStringW(hInstance, IDC_FASTSWITCHAUDIO, g_trayWindowClassName, maxLoadString);

	const WNDCLASSEXW windowClass
	{
		.cbSize = sizeof(WNDCLASSEX),
		.lpfnWndProc = TrayWindowProc,
		.hInstance = hInstance,
		.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_FASTSWITCHAUDIO)),
		.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME),
		.lpszMenuName = MAKEINTRESOURCEW(IDC_FASTSWITCHAUDIO),
		.lpszClassName = g_trayWindowClassName,
		.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_SMALL)),
	};

	RegisterClassExW(&windowClass);

	g_trayWindow = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		g_trayWindowClassName, L"", WS_POPUP,
		0, 0, 0, 0, NULL, NULL, hInstance, NULL
	);

	if (g_trayWindow == NULL)
	{
		ShowFatalError(L"Failed to create the tray window!");
		return FALSE;
	}

	HMENU popupMenuHandle = CreatePopupMenu();

	if (popupMenuHandle == NULL)
	{
		ShowFatalError(L"Failed to create a popup context menu!");
		return FALSE;
	}
	
	g_popupMenu = new UI::PopupMenu<MenuItemData>(g_trayWindow, popupMenuHandle);
	g_audioDeviceManager = new Audio::DeviceManager([]
	{
		if (!g_awaitingRefresh)
		{
			g_awaitingRefresh = true;
			PostMessageW(g_trayWindow, static_cast<UINT>(Notification::RefreshItems), 0, 0);
		}
	}, ShowFatalError);

	if (auto error = g_audioDeviceManager->refresh())
	{
		ShowFatalError(error->explanation);
		return FALSE;
	}

	NOTIFYICONDATAW trayIconData{
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

void RefreshPopupMenu()
{
	g_popupMenu->clear();

	std::array<const Audio::Device*, 3> defaultDevices {
		g_audioDeviceManager->getDefault(eConsole),
		g_audioDeviceManager->getDefault(eMultimedia),
		g_audioDeviceManager->getDefault(eCommunications)
	};

	for (const Audio::Device& device : g_audioDeviceManager->devices)
	{
		bool isDefault = false;

		for (const Audio::Device* defaultDevice : defaultDevices)
		{
			if (defaultDevice == NULL)
			{
				continue;
			}

			if (defaultDevice->id != device.id)
			{
				continue;
			}

			isDefault = true;
			break;
		}

		std::wstring label = std::wstring(device.getName());

		// TODO: C++ may be sneakily doing the whole copy constructor business here, I haven't checked though.
		g_popupMenu->append(MenuItemData::device(device.id), label, isDefault);
	}

	g_popupMenu->appendSeparator();
	g_popupMenu->append(MenuItemData::button(MenuItemData::Type::RefreshButton), L"Refresh");
	g_popupMenu->append(MenuItemData::button(MenuItemData::Type::ExitButton), L"Quit");
}

LRESULT CALLBACK TrayWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
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

					MenuItemData item = maybeItem.value();

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
							if (auto error = g_audioDeviceManager->refresh())
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

		case static_cast<UINT>(Notification::RefreshItems):
		{
			RefreshPopupMenu();
			g_awaitingRefresh = false;

			break;
		}

		case WM_DESTROY:
		{
			NOTIFYICONDATAW trayIconData{
				.cbSize = sizeof(NOTIFYICONDATAW),
				.hWnd = g_trayWindow,
				.uID = trayIconId
			};

			Shell_NotifyIconW(NIM_DELETE, &trayIconData);

			delete g_audioDeviceManager;
			delete g_popupMenu;

			//PostQuitMessage(0);
			break;
		}

		default:
		{
			return DefWindowProcW(window, message, wParam, lParam);
		}

	}

	return 0;
}
