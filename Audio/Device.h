
#pragma once

#include "Audio.h"

namespace Audio
{

	class Device
	{
	public:

		Device(IMMDevice* mmDevice, IPropertyStore* propertyStore, std::wstring id);
		Device(Device&& device) noexcept;
		~Device();

		const std::wstring getName() const;
		void setAsDefault();

		IMMDevice* mmDevice;
		IPropertyStore* propertyStore;
		const std::wstring id;

	};

};
