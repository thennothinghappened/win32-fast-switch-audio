
#pragma once

#include "Audio.h"
#include "Device.h"
#include <vector>

namespace Audio
{
	class DeviceManager
	{

	public:

		/**
		 * @brief Refreshes the list of audio devices.
		 * @return An error, if one occurred during the operation.
		 */
		[[nodiscard]]
		const std::optional<Error> refresh();

		/**
		 * @brief Gets an audio device by its unique ID.
		 * @param id Opaque identifier of the audio device.
		 * @return The audio device associated with the provided ID.
		 */
		Device& operator[](Device::Id id);

		/**
		 * @brief Gets the audio device that's been set as the default.
		 * @return The default audio device.
		 */
		Device& getDefault();

		std::vector<Device> devices;

	private:
		IMMDeviceEnumerator* deviceEnumerator = NULL;

	};
};
