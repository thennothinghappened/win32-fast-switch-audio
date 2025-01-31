#pragma once

#include "framework.h"
#include <vector>
#include <string>
#include <optional>

class ListView
{

public:
	ListView(const HWND parent);
	~ListView();

	void updateSize();
	void clear();
	void insert(std::int32_t index, std::wstring item);

	/**
	 * @brief Handles the given window notification event, if it is ours.
	 * @returns Whether the notification belonged to this view, and thusly if it was taken. (FIXME: temporarily changed to return list index (if any))
	 */
	std::optional<std::int32_t> handleNotification(const NMHDR* notification) const;

private:
	const HWND listView;

};
