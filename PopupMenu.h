
#pragma once

#include "framework.h"
#include <unordered_map>
#include <string>
#include <format>
#include <optional>
#include <stdexcept>

template<typename ITEM>
class PopupMenu
{
public:
	
	PopupMenu(HWND owner, HMENU menu) 
		: owner(owner), menu(menu) {}

	/**
	 * @brief Append a new item to the end of the item list.
	 * @param item The item to append.
	 * @param label A label displayed for the item.
	 */
	void append(ITEM item, std::wstring label)
	{
		UINT id = 1;

		while (items.contains(id))
		{
			id++;
		}

		items.insert({ id, item });
		AppendMenuW(menu, MF_ENABLED | MF_STRING, id, label.data());
	}

	/**
	 * @brief Clear the list of items in the menu.
	 */
	void clear()
	{
		for (auto iterator = items.cbegin(); iterator != items.cend(); iterator++)
		{
			UINT id = iterator->first;
			RemoveMenu(menu, id, MF_BYCOMMAND);
		}

		items.clear();
	}

	/**
	 * @brief Analogous to Win32's `TrackPopupMenu()`, but returns the assocated item directly, instead of the
	 * @param position The position to show the menu at.
	 * @return The chosen item, or nothing if no option was chosen.
	 */
	std::optional<ITEM> track(POINT position) const
	{
		const UINT id = TrackPopupMenu(menu,
			TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			position.x,
			position.y,
			0, owner, NULL
		);

		if (id == 0)
		{
			return std::nullopt;
		}

		return items.at(id);
	}

private:
	const HMENU menu;
	const HWND owner;

	/**
	 * @brief Menu items in the popup context menu, associated with IDs.
	 */
	std::unordered_map<UINT, ITEM> items;

};
