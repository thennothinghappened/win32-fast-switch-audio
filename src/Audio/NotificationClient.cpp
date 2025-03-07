
#include "DeviceManager.h"

using namespace Audio;

HRESULT __stdcall DeviceManager::NotificationClient::OnDeviceStateChanged(LPCWSTR, DWORD)
{
	if (const auto maybeError = deviceManager->refresh())
	{
		deviceManager->onFatalError(maybeError->explanation);
	}

	return S_OK;
}

HRESULT __stdcall DeviceManager::NotificationClient::OnDeviceAdded(LPCWSTR)
{
	if (const auto maybeError = deviceManager->refresh())
	{
		deviceManager->onFatalError(maybeError->explanation);
	}

	return S_OK;
}

HRESULT __stdcall DeviceManager::NotificationClient::OnDeviceRemoved(LPCWSTR)
{
	if (const auto maybeError = deviceManager->refresh())
	{
		deviceManager->onFatalError(maybeError->explanation);
	}

	return S_OK;
}

HRESULT __stdcall DeviceManager::NotificationClient::OnDefaultDeviceChanged(
	EDataFlow,
	ERole,
	LPCWSTR
)
{
	if (const auto maybeError = deviceManager->refresh())
	{
		deviceManager->onFatalError(maybeError->explanation);
	}

	return S_OK;
}

HRESULT __stdcall DeviceManager::NotificationClient::OnPropertyValueChanged(
	LPCWSTR,
	const PROPERTYKEY
)
{
	return S_OK;
}

HRESULT __stdcall DeviceManager::NotificationClient::QueryInterface(REFIID riid, void** ppvObject)
{
	if (riid == __uuidof(IMMNotificationClient))
	{
		*ppvObject = static_cast<IMMNotificationClient*>(this);
		goto success;
	}

	if (riid == __uuidof(IUnknown))
	{
		*ppvObject = static_cast<IUnknown*>(this);
		goto success;
	}

	*ppvObject = nullptr;
	return E_NOINTERFACE;

success:
	AddRef();
	return S_OK;
}

ULONG __stdcall DeviceManager::NotificationClient::AddRef()
{
	refCount++;
	return refCount;
}

ULONG __stdcall DeviceManager::NotificationClient::Release()
{
	refCount--;

	if (refCount == 0)
	{
		delete this;
		return 0;
	}

	return refCount;
}
