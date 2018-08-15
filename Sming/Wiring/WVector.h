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

#ifndef WVECTOR_H
#define WVECTOR_H

#include "Countable.h"
#include <stdlib.h>
#include <string.h>

template <typename Element> class Vector : public Countable<Element> {
public:
	typedef int (*Comparer)(const Element& lhs, const Element& rhs);

	// constructors
	Vector(unsigned initialCapacity = 10, unsigned capacityIncrement = 10);
	Vector(const Vector& rhv);
	virtual ~Vector();

	// methods
	unsigned capacity() const;
	bool contains(const Element& elem) const;
	const Element& firstElement() const;
	int indexOf(const Element& elem) const;
	bool isEmpty() const;
	const Element& lastElement() const;
	int lastIndexOf(const Element& elem) const;

	unsigned count() const
	{
		return size();
	}

	unsigned size() const;
	void copyInto(Element* array) const;

	bool add(const Element& obj)
	{
		addElement(obj);
		return true;
	}

	void addElement(const Element& obj);
	void addElement(Element* objp);

	void clear()
	{
		removeAllElements();
	}

	void ensureCapacity(unsigned minCapacity);
	void removeAllElements();
	bool removeElement(const Element& obj);
	void setSize(unsigned newSize);
	void trimToSize();
	const Element& elementAt(unsigned index) const;
	void insertElementAt(const Element& obj, unsigned index);
	const void remove(unsigned index);
	void removeElementAt(unsigned index);
	void setElementAt(const Element& obj, unsigned index);

	inline const Element& get(unsigned index) const
	{
		return elementAt(index);
	}

	const Element& operator[](unsigned index) const;
	Element& operator[](unsigned index);

	const Vector<Element>& operator=(const Vector<Element>& rhv)
	{
		if (this != &rhv)
			copyFrom(rhv);
		return *this;
	}

	const Vector<Element>& operator=(const Vector<Element>&& other) // move assignment
	{
		if (_data) {
			removeAllElements();
			delete[] _data;
		}
		// move
		_data = other._data;
		_size = other._size;
		_capacity = other._capacity;
		_increment = other._increment;
		// leave moved-from in valid state
		other._data = nullptr;
		other._size = 0;
		other._capacity = 0;
		other._increment = 0;
		return *this;
	}

	void sort(Comparer compareFunction);

protected:
	void copyFrom(const Vector& rhv);

protected:
	unsigned _size = 0;
	unsigned _capacity = 0;
	unsigned _increment = 0;
	Element** _data = nullptr;
};

template <class Element> Vector<Element>::Vector(unsigned initialCapacity, unsigned capacityIncrement)
{
	_size = 0;
	_capacity = initialCapacity;
	_data = new Element*[_capacity];
	_increment = capacityIncrement;
	if (_data == NULL)
		_capacity = _increment = 0;
}

template <class Element> Vector<Element>::Vector(const Vector<Element>& rhv)
{
	copyFrom(rhv);
}

template <class Element> void Vector<Element>::copyFrom(const Vector<Element>& rhv)
{
	if (_data) {
		removeAllElements();
		delete[] _data;
	}
	_size = rhv._size;
	_capacity = rhv._capacity;
	_data = new Element*[_capacity];
	_increment = rhv._increment;
	if (_data == NULL)
		_size = _capacity = _increment = 0;

	for (unsigned i = 0; i < _size; i++)
		_data[i] = new Element(*(rhv._data[i]));
}

template <class Element> Vector<Element>::~Vector()
{
	removeAllElements();
	delete[] _data;
}

template <class Element> unsigned Vector<Element>::capacity() const
{
	return _capacity;
}

template <class Element> bool Vector<Element>::contains(const Element& elem) const
{
	return indexOf(elem) >= 0;
}

template <class Element> void Vector<Element>::copyInto(Element* array) const
{
	if (array)
		for (unsigned i = 0; i < _size; i++)
			array[i] = *_data[i];
}

template <class Element> const Element& Vector<Element>::elementAt(unsigned index) const
{
	//static Element dummy_writable_element;
	if (index >= _size || !_data) {
		//dummy_writable_element = 0;
		//return dummy_writable_element;
		abort();
	}
	// add check for valid index
	return *_data[index];
}

template <class Element> const Element& Vector<Element>::firstElement() const
{
	//static Element dummy_writable_element;
	if (_size == 0 || !_data) {
		//dummy_writable_element = 0;
		//return dummy_writable_element;
		abort();
	}

	return *_data[0];
}

template <class Element> int Vector<Element>::indexOf(const Element& elem) const
{
	for (unsigned i = 0; i < _size; i++)
		if (*_data[i] == elem)
			return i;

	return -1;
}

template <class Element> bool Vector<Element>::isEmpty() const
{
	return _size == 0;
}

template <class Element> const Element& Vector<Element>::lastElement() const
{
	//static Element dummy_writable_element;
	if (_size == 0 || !_data) {
		//dummy_writable_element = 0;
		//return dummy_writable_element;
		abort();
	}

	return *_data[_size - 1];
}

template <class Element> int Vector<Element>::lastIndexOf(const Element& elem) const
{
	//  check for empty vector
	if (_size == 0)
		return -1;

	unsigned i = _size;
	while (--i)
		if (*_data[i] == elem)
			return i;

	return -1;
}

template <class Element> unsigned Vector<Element>::size() const
{
	return _size;
}

template <class Element> void Vector<Element>::addElement(const Element& obj)
{
	if (_size == _capacity)
		ensureCapacity(_capacity + _increment);
	if (_size < _capacity)
		_data[_size++] = new Element(obj);
}

template <class Element> void Vector<Element>::addElement(Element* objp)
{
	if (_size == _capacity)
		ensureCapacity(_capacity + _increment);
	if (_size < _capacity)
		_data[_size++] = objp;
}

template <class Element> void Vector<Element>::ensureCapacity(unsigned minCapacity)
{
	if (_capacity >= minCapacity)
		return;

	//_capacity = minCapacity;
	Element** temp = new Element*[minCapacity];

	// copy all elements
	if (temp) {
		_capacity = minCapacity;
		memcpy(temp, _data, sizeof(Element*) * _size);
		delete[] _data;
		_data = temp;
	}
}

template <class Element> void Vector<Element>::insertElementAt(const Element& obj, unsigned index)
{
	if (index == _size) {
		addElement(obj);
		return;
	}

	//  need to verify index, right now you must know what you're doing
	if (index > _size)
		return;

	if (_size == _capacity)
		ensureCapacity(_capacity + _increment);

	if (_size < _capacity) {
		Element* newItem = new Element(obj); //  pointer to new item
		Element* tmp;						 // temp to hold item to be moved over

		for (unsigned i = index; i <= _size; i++) {
			tmp = _data[i];
			_data[i] = newItem;

			if (i == _size)
				break;

			newItem = tmp;
		}
		_size++;
	}
}

template <class Element> const void Vector<Element>::remove(unsigned index)
{
	removeElementAt(index);
}

template <class Element> void Vector<Element>::removeAllElements()
{
	// avoid memory leak
	for (unsigned i = 0; i < _size; i++)
		delete _data[i];

	_size = 0;
}

template <class Element> bool Vector<Element>::removeElement(const Element& obj)
{
	for (unsigned i = 0; i < _size; i++) {
		if (*_data[i] == obj) {
			removeElementAt(i);
			return true;
		}
	}
	return false;
}

template <class Element> void Vector<Element>::removeElementAt(unsigned index)
{
	// check for valid index
	if (index >= _size)
		return;

	delete _data[index];

	unsigned i;
	for (i = index + 1; i < _size; i++)
		_data[i - 1] = _data[i];

	// ???
	//  _data[i];

	_size--;
}

template <class Element> void Vector<Element>::setElementAt(const Element& obj, unsigned index)
{
	// check for valid index
	if (index >= _size)
		return;
	*_data[index] = obj;
}

template <class Element> void Vector<Element>::setSize(unsigned newSize)
{
	if (newSize > _capacity)
		ensureCapacity(newSize);
	else if (newSize < _size) {
		for (unsigned i = newSize; i < _size; i++)
			delete _data[i];

		_size = newSize;
	}
}

template <class Element> void Vector<Element>::trimToSize()
{
	if (_size != _capacity) {
		Element** temp = new Element*[_size];
		if (!temp)
			return;

		for (unsigned i = 0; i < _size; i++)
			temp[i] = _data[i];

		delete[] _data;

		_data = temp;
		_capacity = _size;
	}
}

template <class Element> const Element& Vector<Element>::operator[](unsigned index) const
{
	return elementAt(index);
}

template <class Element> Element& Vector<Element>::operator[](unsigned index)
{
	// check for valid index
	//static Element dummy_writable_element;
	if (index >= _size || !_data) {
		//dummy_writable_element = 0;
		//return dummy_writable_element;
		abort();
	}
	return *_data[index];
}

template <class Element> void Vector<Element>::sort(Comparer compareFunction)
{
	int i, j;
	// Start with 1 (not 0)
	for (j = 1; j < _size; j++) {
		Element* key = _data[j];
		// Smaller values move up
		for (i = j - 1; (i >= 0) && compareFunction(*_data[i], *key) > 0; i--)
			_data[i + 1] = _data[i];
		// Put key into its proper location
		_data[i + 1] = key;
	}
}

#endif // WVECTOR_H
