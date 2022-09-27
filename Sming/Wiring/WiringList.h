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

	bool allocate(size_t newSize);

	void clear()
	{
		free(values);
		values = nullptr;
		size = 0;
	}

	bool insert(unsigned index, T value)
	{
		memmove(&values[index + 1], &values[index], size - index - 1);
		values[index] = value;
		return true;
	}

	void remove(unsigned index)
	{
		memmove(&values[index], &values[index + 1], (size - index - 1) * sizeof(T));
	}

	void trim(size_t newSize, bool reallocate)
	{
		if(!reallocate) {
			return;
		}

		auto newmem = realloc(values, sizeof(T) * newSize);
		if(newmem == nullptr) {
			return;
		}

		values = static_cast<T*>(newmem);
		size = newSize;
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
template <typename T> struct ObjectList : public ScalarList<T*> {
	struct Element {
		T*& value;

		Element& operator=(T* v)
		{
			delete value;
			value = v;
			return *this;
		}

		template <typename U = T>
		typename std::enable_if<std::is_copy_constructible<U>::value, Element&>::type operator=(const U& v)
		{
			delete value;
			value = new U{v};
			return *this;
		}

		operator T&()
		{
			return *value;
		}
	};

	~ObjectList()
	{
		clear();
	}

	bool allocate(size_t newSize);

	void clear()
	{
		while(this->size != 0) {
			delete this->values[--this->size];
		}
		ScalarList<T*>::clear();
	}

	bool insert(unsigned index, const T& value)
	{
		auto el = new T(value);
		if(el == nullptr) {
			return false;
		}
		return ScalarList<T*>::insert(index, el);
	}

	void remove(unsigned index)
	{
		delete this->values[index];
		ScalarList<T*>::remove(index);
		this->values[this->size - 1] = nullptr;
	}

	void trim(size_t newSize, bool reallocate)
	{
		for(unsigned i = this->size; i > newSize; --i) {
			delete this->values[i - 1];
			this->values[i - 1] = nullptr;
		}

		ScalarList<T*>::trim(newSize, reallocate);
	}

	Element operator[](unsigned index)
	{
		return Element{this->values[index]};
	}

	const T& operator[](unsigned index) const
	{
		return *this->values[index];
	}
};

template <typename T> bool ScalarList<T>::allocate(size_t newSize)
{
	if(newSize <= size) {
		return true;
	}

	auto newmem = realloc(values, sizeof(T) * newSize);
	if(newmem == nullptr) {
		return false;
	}

	values = static_cast<T*>(newmem);
	size = newSize;
	return true;
}

template <typename T> bool ObjectList<T>::allocate(size_t newSize)
{
	auto curSize = this->size;
	if(!ScalarList<T*>::allocate(newSize)) {
		return false;
	}

	std::fill_n(&this->values[curSize], newSize - curSize, nullptr);
	return true;
}

template <typename T>
using List = typename std::conditional<std::is_scalar<T>::value, ScalarList<T>, ObjectList<T>>::type;

} // namespace wiring_private
