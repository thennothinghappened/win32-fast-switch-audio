
#include "DeviceImpl.h"
#include <functiondiscoverykeys_devpkey.h>

using namespace Audio;

DeviceImpl::DeviceImpl(IMMDevice* mmDevice, IPropertyStore* propertyStore, std::wstring id)
	: mmDevice(mmDevice), propertyStore(propertyStore), id(id)
{
	mmDevice->AddRef();
	propertyStore->AddRef();
}

DeviceImpl::DeviceImpl(DeviceImpl&& device) noexcept : mmDevice(device.mmDevice), propertyStore(device.propertyStore), id(device.id)
{
	mmDevice->AddRef();
	propertyStore->AddRef();
}

DeviceImpl::~DeviceImpl()
{
	propertyStore->Release();
	mmDevice->Release();
}

const std::wstring DeviceImpl::getName() const
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

void DeviceImpl::setAsDefault()
{
	CComPtr<IPolicyConfig> policyConfig;
	policyConfig.CoCreateInstance(__uuidof(CPolicyConfigClient), NULL, CLSCTX_ALL); // TODO: error handling
	policyConfig->SetDefaultEndpoint(id.data(), eConsole);
}
