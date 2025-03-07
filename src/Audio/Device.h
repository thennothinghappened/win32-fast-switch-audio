
#pragma once

#include "../framework.h"

#include "Audio.h"

namespace Audio
{
	class Device
	{
	public:
		/**
		 * @brief Audio devices have opaque IDs that are used to reference them, but we aren't meant to look
		 * at what they actually hold.
		 */
		using Id = const std::wstring;

		Device(IMMDevice* mmDevice, IPropertyStore* propertyStore, const std::wstring& id);

		Device(Device&& device) noexcept;

		~Device();

		std::wstring getName() const;

		Id id;

	private:
		IMMDevice* mmDevice;
		IPropertyStore* propertyStore;
	};
};
