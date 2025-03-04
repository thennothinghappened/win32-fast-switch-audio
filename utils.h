
#pragma once
#include "framework.h"
#include <string>

/**
 * @brief Show a fatal error message box if something went wrong we cannot recover from.
 * @param message The message to be shown.
 */
void ShowFatalError(const std::wstring message)
{
	FatalAppExitW(0, message.c_str());
}
