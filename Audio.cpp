#include "Audio.h"
#include <format>
#include <exception>
#include <functiondiscoverykeys_devpkey.h>

namespace Audio
{

	DeviceManager::DeviceManager() {}

	std::optional<Error> DeviceManager::refresh()
	{
		devices.clear();

		if (deviceEnumerator == nullptr)
		{
			if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&deviceEnumerator))))
			{
				return Error{ L"Failed to get an audio device enumerator instance" };
			}
		}

		CComPtr<IMMDeviceCollection> mmOutputs;

		if (FAILED(deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &mmOutputs)))
		{
			return Error{ L"Failed to retrieve list of audio output devices" };
		}

		std::uint32_t outputCount;

		if (FAILED(mmOutputs->GetCount(&outputCount)))
		{
			return Error{ L"Failed to count # of audio output devices" };
		}

		devices.reserve(outputCount);

		for (std::uint32_t i = 0; i < outputCount; i++)
		{

			IMMDevice* mmDevice;
			IPropertyStore* propertyStore;

			if (FAILED(mmOutputs->Item(i, &mmDevice)))
			{
				return Error{ std::format(L"Failed to retrieve the device at index {}", i) };
			}

			if (FAILED(mmDevice->OpenPropertyStore(STGM_READ, &propertyStore)))
			{
				mmDevice->Release();
				return Error{ std::format(L"Failed to open property store for device at index {}", i) };
			}

			devices.emplace_back(Device(mmDevice, propertyStore));

			// Release our references, since the device adds one.
			mmDevice->Release();
			propertyStore->Release();

		}

		return std::nullopt;

	}

	Device::Device(IMMDevice* mmDevice, IPropertyStore* propertyStore)
		: mmDevice(mmDevice), propertyStore(propertyStore)
	{
		mmDevice->AddRef();
		propertyStore->AddRef();
	}

	Device::Device(Device&& device) noexcept : mmDevice(device.mmDevice), propertyStore(device.propertyStore)
	{
		mmDevice->AddRef();
		propertyStore->AddRef();
	}

	Device::~Device()
	{
		propertyStore->Release();
		mmDevice->Release();
	}

	std::wstring Device::getName()
	{

		PROPVARIANT shortName;
		PROPVARIANT longName;

		// TODO: error handling? maybe.
		PropVariantInit(&shortName);
		PropVariantInit(&longName);

		propertyStore->GetValue(PKEY_Device_DeviceDesc, &shortName);
		propertyStore->GetValue(PKEY_DeviceInterface_FriendlyName, &longName);

		std::wstring name = std::format(L"{} ({})", shortName.pwszVal, longName.pwszVal);

		PropVariantClear(&shortName);
		PropVariantClear(&longName);

		return name;

	}

};
