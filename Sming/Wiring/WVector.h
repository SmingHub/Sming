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
#include <stdlib.h>
#include <string.h>

template <typename Element> class Vector : public Countable<Element>
{
public:
	typedef int (*Comparer)(const Element& lhs, const Element& rhs);

	// constructors
	Vector(unsigned int initialCapacity = 10, unsigned int capacityIncrement = 10);
	Vector(const Vector& rhv);
	~Vector();

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
	const void remove(unsigned int index);
	void removeElementAt(unsigned int index);
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
	const Vector<Element>& operator=(const Vector<Element>&& other) // move assignment
	{
		if(_data != nullptr) {
			removeAllElements();
			delete[] _data; // delete this storage
		}
		_data = other._data; // move
		_size = other._size;
		_capacity = other._capacity;
		_increment = other._increment;
		other._data = nullptr; // leave moved-from in valid state
		other._size = 0;
		other._capacity = 0;
		other._increment = 0;
		return *this;
	}

	void sort(Comparer compareFunction);

protected:
	void copyFrom(const Vector& rhv);

protected:
	unsigned int _size = 0;
	unsigned int _capacity = 0;
	unsigned int _increment;
	Element** _data = nullptr;
};

template <class Element> Vector<Element>::Vector(unsigned int initialCapacity, unsigned int capacityIncrement)
{
	_size = 0;
	_capacity = initialCapacity;
	_data = new Element*[_capacity];
	_increment = capacityIncrement;
	if(_data == nullptr) {
		_capacity = _increment = 0;
	}
}

template <class Element> Vector<Element>::Vector(const Vector<Element>& rhv)
{
	copyFrom(rhv);
}

template <class Element> void Vector<Element>::copyFrom(const Vector<Element>& rhv)
{
	if(_data != nullptr) {
		removeAllElements();
		delete[] _data;
	}
	_size = rhv._size;
	_capacity = rhv._capacity;
	_data = new Element*[_capacity];
	_increment = rhv._increment;
	if(_data == nullptr) {
		_size = _capacity = _increment = 0;
	}

	for(unsigned int i = 0; i < _size; i++) {
		_data[i] = new Element(*(rhv._data[i]));
	}
}

template <class Element> Vector<Element>::~Vector()
{
	removeAllElements();
	delete[] _data;
}

template <class Element> unsigned int Vector<Element>::capacity() const
{
	return _capacity;
}

template <class Element> bool Vector<Element>::contains(const Element& elem) const
{
	return indexOf(elem) >= 0;
}

template <class Element> void Vector<Element>::copyInto(Element* array) const
{
	if(array != nullptr) {
		for(unsigned int i = 0; i < _size; i++) {
			array[i] = *_data[i];
		}
	}
}

template <class Element> const Element& Vector<Element>::elementAt(unsigned int index) const
{
	if(index >= _size || !_data) {
		abort();
	}
	// add check for valid index
	return *_data[index];
}

template <class Element> const Element& Vector<Element>::firstElement() const
{
	if(_size == 0 || !_data) {
		abort();
	}

	return *_data[0];
}

template <class Element> int Vector<Element>::indexOf(const Element& elem) const
{
	for(unsigned int i = 0; i < _size; i++) {
		if(*_data[i] == elem) {
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
	if(_size == 0 || !_data) {
		abort();
	}

	return *_data[_size - 1];
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
		if(*_data[i] == elem) {
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
	_data[_size++] = new Element(obj);
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
	if(_capacity >= minCapacity) {
		return true;
	}

	auto newCapacity = std::max(minCapacity, _capacity + _increment);
	Element** temp = new Element*[newCapacity];
	// copy all elements
	if(temp == nullptr) {
		return false;
	}

	_capacity = newCapacity;
	memcpy(temp, _data, sizeof(Element*) * _size);
	delete[] _data;
	_data = temp;
	return true;
}

template <class Element> bool Vector<Element>::insertElementAt(const Element& obj, unsigned int index)
{
	if(index == _size) {
		return addElement(obj);
	}

	//  need to verify index, right now you must know what you're doing
	if(index > _size) {
		return false;
	}
	if(!ensureCapacity(_size + 1)) {
		return false;
	}

	Element* newItem = new Element(obj); //  pointer to new item
	if(newItem == nullptr) {
		return false;
	}

	for(unsigned int i = index; i <= _size; i++) {
		Element* tmp = _data[i];
		_data[i] = newItem;

		if(i != _size) {
			newItem = tmp;
		} else {
			break;
		}
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
	// avoid memory leak
	for(unsigned int i = 0; i < _size; i++) {
		delete _data[i];
	}

	_size = 0;
}

template <class Element> bool Vector<Element>::removeElement(const Element& obj)
{
	for(unsigned int i = 0; i < _size; i++) {
		if(*_data[i] == obj) {
			removeElementAt(i);
			return true;
		}
	}
	return false;
}

template <class Element> void Vector<Element>::removeElementAt(unsigned int index)
{
	// check for valid index
	if(index >= _size) {
		return;
	}

	delete _data[index];

	unsigned int i;
	for(i = index + 1; i < _size; i++) {
		_data[i - 1] = _data[i];
	}

	_size--;
}

template <class Element> bool Vector<Element>::setElementAt(const Element& obj, unsigned int index)
{
	// check for valid index
	if(index >= _size) {
		return false;
	}
	*_data[index] = obj;
	return true;
}

template <class Element> bool Vector<Element>::setSize(unsigned int newSize)
{
	if(!ensureCapacity(newSize)) {
		return false;
	}

	if(newSize < _size) {
		for(unsigned int i = newSize; i < _size; i++) {
			delete _data[i];
		}

		_size = newSize;
	}

	return true;
}

template <class Element> void Vector<Element>::trimToSize()
{
	if(_size != _capacity) {
		Element** temp = new Element*[_size];
		if(temp == nullptr) {
			return;
		}

		for(unsigned int i = 0; i < _size; i++) {
			temp[i] = _data[i];
		}

		delete[] _data;

		_data = temp;
		_capacity = _size;
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
	if(index >= _size || !_data) {
		//dummy_writable_element = 0;
		//return dummy_writable_element;
		abort();
	}
	return *_data[index];
}

template <class Element> void Vector<Element>::sort(Comparer compareFunction)
{
	for(unsigned j = 1; j < _size; j++) // Start with 1 (not 0)
	{
		Element* key = _data[j];
		int i;
		for(i = j - 1; (i >= 0) && compareFunction(*_data[i], *key) > 0; i--) // Smaller values move up
		{
			_data[i + 1] = _data[i];
		}
		_data[i + 1] = key; //Put key into its proper location
	}
}
