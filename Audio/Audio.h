#pragma once

#include "../framework.h"
#include <mmdeviceapi.h>
#include <atlbase.h>
#include <optional>
#include <string>
#include <format>
#include <vector>
#include "../PolicyConfig.h"

namespace Audio
{
	struct Error
	{
		std::wstring explanation;
	};

	class Device
	{
	public:
		virtual const std::wstring getName() = 0;
		virtual void setAsDefault() = 0;
	};

	class DeviceImpl : public Device
	{
	public:
		DeviceImpl(IMMDevice* mmDevice, IPropertyStore* propertyStore);
		DeviceImpl(DeviceImpl&& device) noexcept;
		~DeviceImpl();

		const std::wstring getName();
		void setAsDefault();

		IMMDevice* mmDevice;
		IPropertyStore* propertyStore;
	};

	class DeviceManager
	{

	public:

		DeviceManager();
		
		[[nodiscard]]
		std::optional<Error> refresh();

		std::vector<DeviceImpl> devices;

		const Device& getDefault() const;

	private:
		IMMDeviceEnumerator* deviceEnumerator = nullptr;

	};

};
