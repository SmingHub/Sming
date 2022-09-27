/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WiringList.h - Private class templates used by HashMap
 *
 ****/

#pragma once

namespace wiring_private
{
/**
 * @brief List of scalar values
 */
template <typename T> struct ScalarList {
	T* values{nullptr};
	size_t size{0};

	~ScalarList()
	{
		clear();
	}

	bool allocate(size_t newSize, const T& nil);

	void clear()
	{
		free(values);
		values = nullptr;
		size = 0;
	}

	void remove(unsigned index)
	{
		memmove(&values[index], &values[index + 1], (size - index - 1) * sizeof(T));
	}

	T& operator[](unsigned index)
	{
		return values[index];
	}

	const T& operator[](unsigned index) const
	{
		return const_cast<ScalarList&>(*this)[index];
	}
};

/**
 * @brief List of object pointers
 */
template <typename T> struct ObjectList {
	T** values{nullptr};
	size_t size{0};

	~ObjectList()
	{
		clear();
	}

	bool allocate(size_t newSize, ...);

	void clear()
	{
		while(size != 0) {
			delete values[--size];
		}
		free(values);
		values = nullptr;
	}

	void remove(unsigned index)
	{
		delete values[index];
		memmove(&values[index], &values[index + 1], (size - index - 1) * sizeof(T*));
		values[size - 1] = nullptr;
	}

	T& operator[](unsigned index)
	{
		auto& ptr = values[index];
		if(ptr == nullptr) {
			ptr = new T{};
		}
		return *ptr;
	}

	const T& operator[](unsigned index) const
	{
		return const_cast<ObjectList&>(*this)[index];
	}
};

template <typename T> bool ScalarList<T>::allocate(size_t newSize, const T& nil)
{
	if(newSize <= size) {
		return true;
	}

	auto newmem = realloc(values, sizeof(T) * newSize);
	if(newmem == nullptr) {
		return false;
	}

	values = static_cast<T*>(newmem);
	std::fill_n(&values[size], newSize - size, nil);
	size = newSize;
	return true;
}

template <typename T> bool ObjectList<T>::allocate(size_t newSize, ...)
{
	if(newSize <= size) {
		return true;
	}

	auto newmem = realloc(values, sizeof(T*) * newSize);
	if(newmem == nullptr) {
		return false;
	}

	values = static_cast<T**>(newmem);
	std::fill_n(&values[size], newSize - size, nullptr);

	size = newSize;
	return true;
}

template <typename T>
using List = typename std::conditional<std::is_scalar<T>::value, ScalarList<T>, ObjectList<T>>::type;

} // namespace wiring_private
