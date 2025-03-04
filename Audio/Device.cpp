
#include "Device.h"
#include <functiondiscoverykeys_devpkey.h>

using namespace Audio;

Device::Device(IMMDevice* mmDevice, IPropertyStore* propertyStore, std::wstring id)
	: mmDevice(mmDevice), propertyStore(propertyStore), id(id)
{
	OutputDebugStringW(std::format(L"Creating new {}!\n", getName()).c_str());
	mmDevice->AddRef();
	propertyStore->AddRef();
}

Device::Device(Device&& device) noexcept
	: mmDevice(device.mmDevice), propertyStore(device.propertyStore), id(device.id)
{
	OutputDebugStringW(std::format(L"Copying {}!\n", getName()).c_str());
	mmDevice->AddRef();
	propertyStore->AddRef();
}

Device::~Device()
{
	OutputDebugStringW(std::format(L"Goodbye from {}!\n", getName()).c_str());
	propertyStore->Release();
	mmDevice->Release();
}

const std::wstring Device::getName() const
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
