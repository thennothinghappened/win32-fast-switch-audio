
#pragma once

#include "framework.h"
#include <unordered_map>
#include <string>
#include <format>
#include <optional>
#include <stdexcept>

namespace UI
{
	template<typename ITEM>
	class PopupMenu
	{
	public:
		PopupMenu(HWND owner, HMENU menu)
			: owner(owner), menu(menu) {}

		~PopupMenu()
		{
			DestroyMenu(menu);
		}

		/**
		 * @brief Append a new item to the end of the item list.
		 * @param item The item to append.
		 * @param label A label displayed for the item.
		 * @param checked Whether the item has a checkmark beside it. TODO: refactor this.
		 */
		void append(ITEM item, std::wstring label, bool checked = false)
		{
			UINT id = nextId;
			nextId++;

			items.insert({ id, item });
			AppendMenuW(menu, MF_ENABLED | MF_STRING, id, label.data());

			if (checked)
			{
				MENUITEMINFOW itemInfo {
					.cbSize = sizeof(MENUITEMINFOW),
					.fMask = MIIM_STATE,
					.fState = MFS_CHECKED
				};

				SetMenuItemInfoW(menu, id, FALSE, &itemInfo);
			}
		}

		/**
		 * @brief Append a visual separator between items.
		 */
		void appendSeparator()
		{
			UINT id = nextId;
			nextId++;

			AppendMenuW(menu, MF_SEPARATOR, id, nullptr);
		}

		/**
		 * @brief Clear the list of items in the menu.
		 */
		void clear()
		{
			while (GetMenuItemCount(menu) > 0)
			{
				DeleteMenu(menu, 0, MF_BYPOSITION);
			}

			nextId = firstId;
			items.clear();
		}

		/**
		 * @brief Analogous to Win32's `TrackPopupMenu()`, but returns the assocated item directly, instead of the internal ID.
		 * @param position The position to show the menu at.
		 * @return The chosen item, or nothing if no option was chosen.
		 */
		std::optional<ITEM> track(POINT position) const
		{
			const UINT id = TrackPopupMenu(
				menu,
				TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
				position.x,
				position.y,
				0,
				owner,
				nullptr
			);

			if (id == 0)
			{
				return std::nullopt;
			}

			if (!items.contains(id))
			{
				// Must be a separator.
				return std::nullopt;
			}

			return items.at(id);
		}

	private:
		const HMENU menu;
		const HWND owner;

		/**
		 * @brief The first ID to be used. Win32 gives special meaning to `0`, so we start at `1`.
		 */
		static constexpr UINT firstId = 1;

		/**
		 * @brief The next identifier to use for the next appended element.
		 */
		UINT nextId = firstId;

		/**
		 * @brief Menu items in the popup context menu, associated with IDs.
		 */
		std::unordered_map<UINT, ITEM> items;
	};
};
