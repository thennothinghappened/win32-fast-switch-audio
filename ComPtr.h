
#ifndef COMPTR_H
#define COMPTR_H

#include "framework.h"

#include <concepts>

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

	ComPtr(const ComPtr& other)
		: ptr(other.ptr)
	{
		if (ptr != nullptr)
		{
			ptr->AddRef();
		}
	}

	ComPtr(ComPtr&& other) noexcept
		: ptr(other.ptr)
	{
		if (ptr != nullptr)
		{
			ptr->AddRef();
		}
	}

	~ComPtr()
	{
		if (ptr != nullptr)
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
	T* ptr = nullptr;
};

#endif
