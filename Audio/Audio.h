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
		virtual const std::wstring getName() const = 0;
		virtual void setAsDefault() = 0;
	};

	class DeviceManager
	{
	public:

		[[nodiscard]]
		virtual std::optional<Error> refresh() = 0;
		virtual Device& operator[](size_t index) = 0;
		virtual Device& getDefault() = 0;
		virtual const size_t count() const = 0;

	};

};
