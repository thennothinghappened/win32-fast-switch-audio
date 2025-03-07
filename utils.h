
#pragma once

#include "framework.h"

#include <string>

#ifdef _DEBUG
#define DEBUG_WRITE(MESSAGE) OutputDebugStringW(std::format(L"{} {} line {}: {}\n", (__FILEW__), (__FUNCTIONW__), (__LINE__), (MESSAGE)).c_str())
#else
#define DEBUG_WRITE(MESSAGE)
#endif

/**
 * @brief Show a fatal error message box if something went wrong we cannot recover from.
 * @param message The message to be shown.
 */
void ShowFatalError(const std::wstring message)
{
	FatalAppExitW(0, message.c_str());
}
