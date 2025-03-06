#pragma once

#include "../framework.h"
#include <mmdeviceapi.h>
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
};
