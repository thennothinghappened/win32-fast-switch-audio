
#pragma once

#include "Audio.h"
#include "DeviceImpl.h"
#include <vector>

namespace Audio
{
	class DeviceManagerImpl final : public DeviceManager
	{

	public:

		[[nodiscard]]
		std::optional<Error> refresh();

		Device& operator[](size_t index);
		Device& getDefault();
		const size_t count() const;

	private:
		std::vector<DeviceImpl> devices;
		IMMDeviceEnumerator* deviceEnumerator = nullptr;

	};
};
