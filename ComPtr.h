#pragma once

#include "framework.h"
#include <cstdint>
#include <concepts>
#include <functional>

/**
 * @brief Replacement for ATL's `CComPtr<T>` which adds a constraint on deriving from `IUnknown`,
 * but mainly to avoid pulling in ATL for one feature.
 * 
 * @tparam T A COM interface.
 */
template<std::derived_from<IUnknown> T>
class ComPtr
{
public:
	ComPtr()
	{}

	ComPtr(ComPtr<T>& other)
		: ptr(other.ptr)
	{
		if (ptr)
		{
			ptr->AddRef();
		}
	}

	ComPtr(ComPtr<T>&& other)
		: ptr(other.ptr)
	{
		if (ptr)
		{
			ptr->AddRef();
		}
	}

	~ComPtr()
	{
		if (ptr)
		{
			ptr->Release();
		}
	}

	T** operator&()
	{
		return &ptr;
	}

	T* operator->()
	{
		return ptr;
	}

private:
	T* ptr = NULL;
};
