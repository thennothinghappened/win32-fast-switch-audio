#include "ListView.h"
#include <CommCtrl.h>
#include <uxtheme.h>

const INITCOMMONCONTROLSEX commonControlsInitData {
	.dwSize = sizeof(INITCOMMONCONTROLSEX),
	.dwICC = ICC_LISTVIEW_CLASSES
};

ListView::ListView(const HWND parent) : listView(CreateWindowW(WC_LISTVIEW, L"",
	WS_CHILD | WS_VISIBLE | LVS_LIST | LVS_SINGLESEL,
	0, 0,
	0, 0,
	parent, nullptr, GetModuleHandleW(nullptr), nullptr
)) {
	InitCommonControlsEx(&commonControlsInitData);
	updateSize();
}

ListView::~ListView() {
	DestroyWindow(listView);
}

void ListView::updateSize() {

	RECT clientRect;
	GetClientRect(GetParent(listView), &clientRect);

	MoveWindow(listView,
		0, 0,
		clientRect.right - clientRect.left,
		clientRect.bottom - clientRect.top,
		true
	);

}

void ListView::clear() {
	ListView_DeleteAllItems(listView);
}

void ListView::insert(std::int32_t index, std::wstring item) {

	const LVITEM listItem {
		.mask = LVIF_TEXT,
		.iItem = index,
		// FIXME: I actually have no idea if this is invalid - the docs are not clear on what happens
		// with `pszText` once I pass it, so I dunno if doing this is doing evil pointer things.
		.pszText = item.data()
	};

	ListView_InsertItem(listView, &listItem);

}
