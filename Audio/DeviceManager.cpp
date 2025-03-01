
#include "DeviceManager.h"

using namespace Audio;

const std::optional<Error> DeviceManager::refresh()
{
	devices.clear();

	if (deviceEnumerator == NULL)
	{
		if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&deviceEnumerator))))
		{
			return Error{ L"Failed to get an audio device enumerator instance" };
		}
	}

	CComPtr<IMMDeviceCollection> mmOutputs;

	if (FAILED(deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &mmOutputs)))
	{
		return Error{ L"Failed to retrieve list of audio output devices" };
	}

	uint32_t outputCount;

	if (FAILED(mmOutputs->GetCount(&outputCount)))
	{
		return Error{ L"Failed to count # of audio output devices" };
	}

	devices.reserve(outputCount);

	for (uint32_t i = 0; i < outputCount; i++)
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

		devices.emplace_back(Device(mmDevice, propertyStore, id));

		// Release our references, since the device adds one.
		mmDevice->Release();
		propertyStore->Release();

	}

	return std::nullopt;

}

Device& Audio::DeviceManager::operator[](Device::Id id)
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

Device& DeviceManager::getDefault()
{
	CComPtr<IMMDevice> mmDevice;
	deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &mmDevice); // TODO: error handling.

	wchar_t* win32Id;
	mmDevice->GetId(&win32Id); // TODO: error handling.
	
	std::wstring id = win32Id;
	CoTaskMemFree(win32Id);

	for (Device& device : devices)
	{
		if (device.id == id)
		{
			return device;
		}
	}

	// FIXME: We really should subscribe to changes to make this situation impossible rather than explicit `refresh()`.
	throw std::logic_error("Couldn't find a match for the default device!");

}
