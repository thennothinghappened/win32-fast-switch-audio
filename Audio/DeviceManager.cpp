
#include "DeviceManager.h"

using namespace Audio;

DeviceManager::DeviceManager(const OnChange onChange, const OnFatalError onFatalError)
	: onChange(onChange), onFatalError(onFatalError)
{
	if (FAILED(CoCreateInstance(__uuidof(CPolicyConfigClient), NULL, CLSCTX_ALL, IID_PPV_ARGS(&policyConfig))))
	{
		throw Error{ L"Failed to get a IPolicyConfig instance, so we cannot actually set the default device" };
	}

	if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&deviceEnumerator))))
	{
		throw Error{ L"Failed to get an audio device enumerator instance" };
	}

	deviceEnumerator->RegisterEndpointNotificationCallback(notificationClient);
}

Audio::DeviceManager::~DeviceManager()
{
	OutputDebugStringW(L"DeviceManager::~DeviceManager()");

	deviceEnumerator->UnregisterEndpointNotificationCallback(notificationClient);
	policyConfig->Release();
	deviceEnumerator->Release();
	notificationClient->Release();
}

const std::optional<Error> DeviceManager::refresh()
{
	CComPtr<IMMDeviceCollection> mmOutputs;
	devices.clear();

	if (FAILED(deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &mmOutputs)))
	{
		return Error{ L"Failed to retrieve list of audio output devices" };
	}

	UINT outputCount;

	if (FAILED(mmOutputs->GetCount(&outputCount)))
	{
		return Error{ L"Failed to count # of audio output devices" };
	}

	devices.reserve(outputCount);

	for (UINT i = 0; i < outputCount; i++)
	{
		IMMDevice* mmDevice;
		IPropertyStore* propertyStore;

		if (FAILED(mmOutputs->Item(i, &mmDevice)))
		{
			return Error{ std::format(L"Failed to retrieve the device at index {}", i) };
		}

		wchar_t* win32Id;

		if (FAILED(mmDevice->GetId(&win32Id)))
		{
			return Error{ std::format(L"Failed to retrieve ID of device at index {}", i) };
		}

		std::wstring id = win32Id;
		CoTaskMemFree(win32Id);

		if (FAILED(mmDevice->OpenPropertyStore(STGM_READ, &propertyStore)))
		{
			mmDevice->Release();
			return Error{ std::format(L"Failed to open property store for device at index {}", i) };
		}

		devices.emplace_back(mmDevice, propertyStore, id);

		// Release our references, since the device adds one.
		mmDevice->Release();
		propertyStore->Release();
	}

	onChange();
	return std::nullopt;
}

const Device& DeviceManager::operator[](Device::Id id) const
{
	for (const Device& device : devices)
	{
		if (device.id == id)
		{
			return device;
		}
	}

	throw std::logic_error("Couldn't find the device associated with the provided ID");
}

Device& DeviceManager::operator[](Device::Id id)
{
	for (Device& device : devices)
	{
		if (device.id == id)
		{
			return device;
		}
	}

	throw std::logic_error("Couldn't find the device associated with the provided ID");
}

const Device& DeviceManager::getDefault(ERole role) const
{
	CComPtr<IMMDevice> mmDevice;
	deviceEnumerator->GetDefaultAudioEndpoint(eRender, role, &mmDevice); // TODO: error handling.

	wchar_t* win32Id;
	mmDevice->GetId(&win32Id); // TODO: error handling.
	
	Device::Id id = win32Id;
	CoTaskMemFree(win32Id);

	return this->operator[](id);
}

std::optional<Error> Audio::DeviceManager::setDefault(const Device& device)
{
	return setDefault(device.id);
}

std::optional<Error> Audio::DeviceManager::setDefault(Device::Id id)
{
	if (SUCCEEDED(policyConfig->SetDefaultEndpoint(id.data(), eConsole))
		&& SUCCEEDED(policyConfig->SetDefaultEndpoint(id.data(), eMultimedia))
		&& SUCCEEDED(policyConfig->SetDefaultEndpoint(id.data(), eCommunications)))
	{
		return std::nullopt;
	}
	
	return Error{ std::format(L"Failed to set the default device to {} for mysterious reasons unknown", id) };
}
