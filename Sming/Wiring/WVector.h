/* $Id: WVector.h 1156 2011-06-07 04:01:16Z bhagman $
||
|| @author         Hernando Barragan <b@wiring.org.co>
|| @url            http://wiring.org.co/
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
|| @contribution   Alexander Brevig <abrevig@wiring.org.co>
||
|| @description
|| | Vector data structure.
|| |
|| | Wiring Common API
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#pragma once

#include "Countable.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <iterator>
#include "WiringList.h"

/**
 * @brief Vector class template
 * @ingroup wiring
 */
template <typename Element> class Vector : public Countable<Element>
{
public:
	using Comparer = int (*)(const Element& lhs, const Element& rhs);

	template <bool is_const> class Iterator
	{
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = Element;
		using difference_type = std::ptrdiff_t;
		using pointer = Element*;
		using reference = Element&;

		using V = typename std::conditional<is_const, const Vector, Vector>::type;
		using E = typename std::conditional<is_const, const Element, Element>::type;

		Iterator(const Iterator&) = default;
		Iterator(Iterator&&) = default;
		Iterator& operator=(const Iterator&) = default;
		Iterator& operator=(Iterator&&) = default;

		Iterator(V& vector, unsigned index) : vector(vector), index(index)
		{
		}

		~Iterator() = default;

		Iterator& operator++()
		{
			++index;
			return *this;
		}

		Iterator operator++(int)
		{
			Iterator tmp(*this);
			++index;
			return tmp;
		}

		Iterator operator+=(size_t distance)
		{
			Iterator tmp(*this);
			index += distance;
			return tmp;
		}

		bool operator==(const Iterator& rhs) const
		{
			return &vector == &rhs.vector && index == rhs.index;
		}

		bool operator!=(const Iterator& rhs) const
		{
			return !operator==(rhs);
		}

		template <typename U = Element> typename std::enable_if<!is_const, U&>::type operator*()
		{
			return vector[index];
		}

		E& operator*() const
		{
			return vector[index];
		}

	private:
		V& vector;
		unsigned index{0};
	};

	Vector(unsigned int initialCapacity = 10, unsigned int capacityIncrement = 10) : _increment(capacityIncrement)
	{
		_data.allocate(initialCapacity);
	}

	Vector(const Vector& rhv)
	{
		copyFrom(rhv);
	}

	Vector(Vector&&) = delete;

	~Vector() = default;

	// methods
	unsigned int capacity() const
	{
		return _data.size;
	}

	template <typename T> bool contains(const T& elem) const
	{
		return indexOf(elem) >= 0;
	}

	const Element& firstElement() const
	{
		if(_size == 0) {
			abort();
		}

		return _data[0];
	}

	template <typename T> int indexOf(const T& elem) const;

	bool isEmpty() const
	{
		return _size == 0;
	}

	const Element& lastElement() const
	{
		if(_size == 0) {
			abort();
		}

		return _data[_size - 1];
	}

	template <typename T> int lastIndexOf(const T& elem) const;

	unsigned int count() const override
	{
		return size();
	}

	unsigned int size() const
	{
		return _size;
	}

	void copyInto(Element* array) const;

	bool add(const Element& obj)
	{
		return addElement(obj);
	}

	bool addElement(const Element& obj);
	bool addElement(Element* objp);

	void clear()
	{
		removeAllElements();
	}

	bool ensureCapacity(size_t minCapacity);

	void removeAllElements()
	{
		_data.clear();
		_size = 0;
	}

	template <typename T> bool removeElement(const T& elem)
	{
		return removeElementAt(indexOf(elem));
	}

	/**
	 * @brief Reduce or increase number of items
	 * @retval true on success, false on memory reallocation failure
	 *
	 * If increasing number of items, new items will be set to current `nil` value.
	 * If reducing number of items, old items will be deleted.
	 */
	bool setSize(unsigned int newSize);

	/**
	 * @brief Reduce capacity to match current size
	 */
	void trimToSize()
	{
		if(_size < _data.size) {
			_data.trim(_size, true);
		}
	}

	const Element& elementAt(unsigned int index) const
	{
		if(index >= _size) {
			abort();
		}
		return _data[index];
	}

	bool insertElementAt(const Element& obj, unsigned int index);

	bool remove(unsigned int index)
	{
		return removeElementAt(index);
	}

	bool removeElementAt(unsigned int index);
	bool setElementAt(const Element& obj, unsigned int index);

	const Element& get(unsigned int index) const
	{
		return elementAt(index);
	}

	const Element& operator[](unsigned int index) const override
	{
		return elementAt(index);
	}

	Element& operator[](unsigned int index) override
	{
		if(index >= _size) {
			abort();
		}
		return _data[index];
	}

	Vector<Element>& operator=(const Vector<Element>& rhv)
	{
		if(this != &rhv) {
			copyFrom(rhv);
		}
		return *this;
	}

	Vector<Element>& operator=(Vector<Element>&& other) noexcept // move assignment
	{
		clear();
		_increment = 0;
		std::swap(_data, other._data);
		std::swap(_size, other._size);
		std::swap(_increment, other._increment);
		return *this;
	}

	void sort(Comparer compareFunction);

	Iterator<false> begin()
	{
		return Iterator<false>(*this, 0);
	}

	Iterator<false> end()
	{
		return Iterator<false>(*this, count());
	}

	const Iterator<true> begin() const
	{
		return Iterator<true>(*this, 0);
	}

	const Iterator<true> end() const
	{
		return Iterator<true>(*this, count());
	}

protected:
	void copyFrom(const Vector& rhv);

protected:
	using ElementList = wiring_private::List<Element>;

	unsigned int _size{0};
	unsigned int _increment{0};
	ElementList _data;
};

template <class Element> void Vector<Element>::copyFrom(const Vector<Element>& rhv)
{
	_data.clear();
	if(!_data.allocate(rhv._data.size)) {
		_size = _increment = 0;
		return;
	}

	_size = rhv._size;
	_increment = rhv._increment;

	for(unsigned int i = 0; i < _size; i++) {
		_data[i] = rhv._data[i];
	}
}

template <class Element> void Vector<Element>::copyInto(Element* array) const
{
	if(array == nullptr) {
		return;
	}

	for(unsigned int i = 0; i < _size; i++) {
		array[i] = _data[i];
	}
}

template <class Element> template <typename T> int Vector<Element>::indexOf(const T& elem) const
{
	for(unsigned int i = 0; i < _size; i++) {
		if(_data[i] == elem) {
			return int(i);
		}
	}

	return -1;
}

template <class Element> template <typename T> int Vector<Element>::lastIndexOf(const T& elem) const
{
	// check for empty vector
	if(_size == 0) {
		return -1;
	}

	unsigned int i = _size;

	do {
		i--;
		if(_data[i] == elem) {
			return int(i);
		}
	} while(i != 0);

	return -1;
}

template <class Element> bool Vector<Element>::addElement(const Element& obj)
{
	if(!ensureCapacity(_size + 1)) {
		return false;
	}
	_data[_size++] = obj;
	return true;
}

template <class Element> bool Vector<Element>::addElement(Element* objp)
{
	if(!ensureCapacity(_size + 1)) {
		return false;
	}
	_data[_size++] = objp;
	return true;
}

template <class Element> bool Vector<Element>::ensureCapacity(size_t minCapacity)
{
	if(_data.size >= minCapacity) {
		return true;
	}

	auto newCapacity = std::max(minCapacity, _data.size + _increment);
	return _data.allocate(newCapacity);
}

template <class Element> bool Vector<Element>::insertElementAt(const Element& obj, unsigned int index)
{
	if(index == _size) {
		return addElement(obj);
	}

	if(index > _size) {
		return false;
	}
	if(!ensureCapacity(_size + 1)) {
		return false;
	}

	if(!_data.insert(index, obj)) {
		return false;
	}

	_size++;
	return true;
}

template <class Element> bool Vector<Element>::removeElementAt(unsigned int index)
{
	// check for valid index
	if(index >= _size) {
		return false;
	}

	_data.remove(index);
	_size--;
	return true;
}

template <class Element> bool Vector<Element>::setElementAt(const Element& obj, unsigned int index)
{
	// check for valid index
	if(index >= _size) {
		return false;
	}
	_data[index] = obj;
	return true;
}

template <class Element> bool Vector<Element>::setSize(unsigned int newSize)
{
	if(!ensureCapacity(newSize)) {
		return false;
	}

	_data.trim(newSize, false);
	_size = std::min(_size, newSize);
	return true;
}

template <class Element> void Vector<Element>::sort(Comparer compareFunction)
{
	// Start with 1 (not 0)
	for(unsigned j = 1; j < _size; j++) {
		auto key = _data.values[j];
		Element& keyRef = _data[j];
		// Smaller values move up
		int i;
		for(i = int(j) - 1; (i >= 0) && compareFunction(_data[i], keyRef) > 0; i--) {
			_data.values[i + 1] = _data.values[i];
		}
		// Put key into its proper location
		_data.values[i + 1] = key;
	}
}
