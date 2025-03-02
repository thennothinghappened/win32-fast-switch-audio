// FastSwitchAudio.cpp : Defines the entry point for the application.
//

#include "FastSwitchAudio.h"

static WCHAR g_trayWindowClassName[maxLoadString];
static HWND g_trayWindow;

static Audio::DeviceManager g_audioDeviceManager;
static PopupMenu<MenuItemData>* g_popupMenu;

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

	g_popupMenu = new PopupMenu<MenuItemData>(g_trayWindow, popupMenuHandle);

	if (const auto maybeError = g_audioDeviceManager.refresh(); maybeError.has_value())
	{
		Audio::Error error = maybeError.value();
		ShowFatalError(error.explanation);

		return FALSE;
	}

	for (const Audio::Device& device : g_audioDeviceManager.devices)
	{
		std::wstring label = std::wstring(device.getName());

		// TODO: C++ may be sneakily doing the whole copy constructor business here, I haven't checked though.
		g_popupMenu->append(MenuItemData::device(device.id), label);
	}

	g_popupMenu->append(MenuItemData::exitButton(), L"Quit");

	NOTIFYICONDATA trayIconData{
		.cbSize = sizeof(NOTIFYICONDATA),
		.hWnd = g_trayWindow,
		.uID = 1,
		.uFlags = NIF_MESSAGE | NIF_ICON,
		.uCallbackMessage = static_cast<UINT>(Notification::ToggleTrayIcon),
		.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_SMALL)),
		.uVersion = NOTIFYICON_VERSION_4
	};
	
	if (!Shell_NotifyIcon(NIM_ADD, &trayIconData))
	{
		ShowFatalError(L"Failed to add a tray icon to the taskbar!");
		return FALSE;
	}

	if (!Shell_NotifyIcon(NIM_SETVERSION, &trayIconData))
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
							Audio::Device& device = g_audioDeviceManager[deviceId];

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
