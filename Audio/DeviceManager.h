
#pragma once

#include "Audio.h"
#include "Device.h"
#include <vector>

namespace Audio
{
	class DeviceManager
	{

	public:

		[[nodiscard]]
		std::optional<Error> refresh();

		Device& operator[](size_t index);
		Device& getDefault();
		const size_t count() const;

	private:
		std::vector<Device> devices;
		IMMDeviceEnumerator* deviceEnumerator = NULL;

	};
};
