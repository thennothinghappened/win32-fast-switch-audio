#pragma once

#include "framework.h"
#include <mmdeviceapi.h>
#include <atlbase.h>
#include <optional>
#include <string>
#include <vector>

namespace Audio {

	struct Error {
		std::wstring explanation;
	};

	class Device {

	public:
		Device(IMMDevice* mmDevice, IPropertyStore* propertyStore);
		~Device();

		std::wstring getName();

	private:
		IMMDevice* mmDevice;
		IPropertyStore* propertyStore;

	};

	class DeviceManager {

	public:

		DeviceManager();
		
		[[nodiscard]]
		std::optional<Error> refresh();

		std::vector<Device> devices;

	private:
		IMMDeviceEnumerator* deviceEnumerator;

	};

};
