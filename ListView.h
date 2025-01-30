#pragma once

#include "framework.h"
#include <vector>
#include <string>

class ListView {

public:
	ListView(const HWND parent);
	~ListView();

	void updateSize();
	void clear();
	void insert(std::int32_t index, std::wstring item);

private:
	const HWND listView;
	
};
