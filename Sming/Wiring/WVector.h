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

	template <bool is_const> class Iterator : public std::iterator<std::random_access_iterator_tag, Element>
	{
	public:
		using V = typename std::conditional<is_const, const Vector, Vector>::type;
		using E = typename std::conditional<is_const, const Element, Element>::type;

		Iterator(const Iterator&) = default;

		Iterator(V& vector, unsigned index) : vector(vector), index(index)
		{
		}

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

	// constructors
	Vector(unsigned int initialCapacity = 10, unsigned int capacityIncrement = 10);
	Vector(const Vector& rhv);

	// methods
	unsigned int capacity() const;
	bool contains(const Element& elem) const;
	const Element& firstElement() const;
	int indexOf(const Element& elem) const;
	bool isEmpty() const;
	const Element& lastElement() const;
	int lastIndexOf(const Element& elem) const;
	unsigned int count() const override
	{
		return size();
	}
	unsigned int size() const;
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
	bool ensureCapacity(unsigned int minCapacity);
	void removeAllElements();
	bool removeElement(const Element& obj);
	bool setSize(unsigned int newSize);
	void trimToSize();
	const Element& elementAt(unsigned int index) const;
	bool insertElementAt(const Element& obj, unsigned int index);
	bool remove(unsigned int index);
	bool removeElementAt(unsigned int index);
	bool setElementAt(const Element& obj, unsigned int index);
	const Element& get(unsigned int index) const
	{
		return elementAt(index);
	}

	const Element& operator[](unsigned int index) const override;
	Element& operator[](unsigned int index) override;

	const Vector<Element>& operator=(const Vector<Element>& rhv)
	{
		if(this != &rhv)
			copyFrom(rhv);
		return *this;
	}
	const Vector<Element>& operator=(Vector<Element>&& other) noexcept // move assignment
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

	unsigned int _size = 0;
	unsigned int _increment;
	ElementList _data;
};

template <class Element> Vector<Element>::Vector(unsigned int initialCapacity, unsigned int capacityIncrement)
{
	_size = 0;
	_data.allocate(initialCapacity);
	_increment = capacityIncrement;
}

template <class Element> Vector<Element>::Vector(const Vector<Element>& rhv)
{
	copyFrom(rhv);
}

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

template <class Element> unsigned int Vector<Element>::capacity() const
{
	return _data.size;
}

template <class Element> bool Vector<Element>::contains(const Element& elem) const
{
	return indexOf(elem) >= 0;
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

template <class Element> const Element& Vector<Element>::elementAt(unsigned int index) const
{
	if(index >= _size) {
		abort();
	}
	// add check for valid index
	return _data[index];
}

template <class Element> const Element& Vector<Element>::firstElement() const
{
	if(_size == 0) {
		abort();
	}

	return _data[0];
}

template <class Element> int Vector<Element>::indexOf(const Element& elem) const
{
	for(unsigned int i = 0; i < _size; i++) {
		if(_data[i] == elem) {
			return i;
		}
	}

	return -1;
}

template <class Element> bool Vector<Element>::isEmpty() const
{
	return _size == 0;
}

template <class Element> const Element& Vector<Element>::lastElement() const
{
	if(_size == 0) {
		abort();
	}

	return _data[_size - 1];
}

template <class Element> int Vector<Element>::lastIndexOf(const Element& elem) const
{
	// check for empty vector
	if(_size == 0) {
		return -1;
	}

	unsigned int i = _size;

	do {
		i--;
		if(_data[i] == elem) {
			return i;
		}
	} while(i != 0);

	return -1;
}

template <class Element> unsigned int Vector<Element>::size() const
{
	return _size;
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

template <class Element> bool Vector<Element>::ensureCapacity(unsigned int minCapacity)
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

template <class Element> const void Vector<Element>::remove(unsigned int index)
{
	removeElementAt(index);
}

template <class Element> void Vector<Element>::removeAllElements()
{
	_data.clear();
	_size = 0;
}

template <class Element> bool Vector<Element>::removeElement(const Element& obj)
{
	return removeElementAt(indexOf(obj));
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

template <class Element> void Vector<Element>::trimToSize()
{
	if(_size < _data.size) {
		_data.trim(_size, true);
	}
}

template <class Element> const Element& Vector<Element>::operator[](unsigned int index) const
{
	return elementAt(index);
}

template <class Element> Element& Vector<Element>::operator[](unsigned int index)
{
	// check for valid index
	//static Element dummy_writable_element;
	if(index >= _size) {
		//dummy_writable_element = 0;
		//return dummy_writable_element;
		abort();
	}
	return _data[index];
}

template <class Element> void Vector<Element>::sort(Comparer compareFunction)
{
	// Start with 1 (not 0)
	for(unsigned j = 1; j < _size; j++) {
		auto key = _data.values[j];
		Element& keyRef = _data[j];
		// Smaller values move up
		int i;
		for(i = j - 1; (i >= 0) && compareFunction(_data[i], keyRef) > 0; i--) {
			_data.values[i + 1] = _data.values[i];
		}
		// Put key into its proper location
		_data.values[i + 1] = key;
	}
}
