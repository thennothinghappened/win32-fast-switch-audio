
#pragma once

#include "Audio.h"

namespace Audio
{

	class DeviceImpl : public Device
	{
	public:

		DeviceImpl(IMMDevice* mmDevice, IPropertyStore* propertyStore, std::wstring id);
		DeviceImpl(DeviceImpl&& device) noexcept;
		~DeviceImpl();

		const std::wstring getName() const;
		void setAsDefault();

		IMMDevice* mmDevice;
		IPropertyStore* propertyStore;
		const std::wstring id;

	};

};
