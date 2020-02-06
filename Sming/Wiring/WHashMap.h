/* $Id: HashMap.h 1198 2011-06-14 21:08:27Z bhagman $
||
|| @author         Alexander Brevig <abrevig@wiring.org.co>
|| @url            http://wiring.org.co/
|| @url            http://alexanderbrevig.com/
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
||
|| @description
|| | Implementation of a HashMap data structure.
|| |
|| | Wiring Cross-platform Library
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

/*
 * @author: 3 Oct 2018 - mikee47 <mike@sillyhouse.net>
 *
 * Modified to use references (const or otherwise) to avoid object copies when used with classes, e.g. String.
 *
 * Note that if the value is a primitive then setNullValue should be called to provide a default value to be
 * used when adding a new unspecified entry, or if a key value is not present. This should not be necessary
 * for object values as the default constructor will be used.
 *
 */

#pragma once

#include <cstdint>

/**
 * @brief HashMap class template
 * @ingroup wiring
 */
template <typename K, typename V> class HashMap
{
public:
	typedef bool (*comparator)(const K&, const K&);

	/*
    || @constructor
    || | Default constructor
    || #
    */
	HashMap()
	{
	}

	/*
    || @constructor
    || | Initialize this HashMap
    || #
    ||
    || @parameter compare optional function for comparing a key against another (for complex types)
    */
	HashMap(comparator compare) : cb_comparator(compare)
	{
	}

	~HashMap()
	{
		clear();
	}

	/*
    || @description
    || | Get the size of this HashMap
    || #
    ||
    || @return The size of this HashMap
    */
	unsigned int count() const
	{
		return currentIndex;
	}

	/*
    || @description
    || | Get a key at a specified index
    || #
    ||
    || @parameter idx the index to get the key at
    ||
    || @return The key at index idx
    */
	const K& keyAt(unsigned int idx) const
	{
		if(idx >= count()) {
			abort();
		}
		return *keys[idx];
	}

	K& keyAt(unsigned int idx)
	{
		if(idx >= count()) {
			abort();
		}
		return *keys[idx];
	}

	/*
    || @description
    || | Get a value at a specified index
    || #
    ||
    || @parameter idx the index to get the value at
    ||
    || @return The value at index idx
    */
	const V& valueAt(unsigned int idx) const
	{
		if(idx >= count()) {
			abort();
		}
		return *values[idx];
	}

	V& valueAt(unsigned int idx)
	{
		if(idx >= count()) {
			abort();
		}
		return *values[idx];
	}

	/*
    || @description
    || | An indexer for accessing and assigning a value to a key
    || | If a key is used that exists, it returns the value for that key
    || | If there exists no value for that key, a nil value is returned
    || |
    || | Note that if the HashMap object is not const, the non-const version
    || | of this operator will be called which will create a default value
    || | for this key. If that behaviour is not desired, then check for the
    || | existence of the key first, using either contains() or indexOf().
    || #
    ||
    || @parameter key the key to get the value for
    ||
    || @return The const value for key
    */
	const V& operator[](const K& key) const
	{
		// Don't create non-existent values
		auto i = indexOf(key);
		return (i >= 0) ? *values[i] : nil;
	}

	/*
    || @description
    || | An indexer for accessing and assigning a value to a key
    || | If a key is used that exists, it returns the value for that key
    || | If there exists no value for that key, the key is added
    || #
    ||
    || @parameter key the key to get the value for
    ||
    || @return The value for key
    */
	V& operator[](const K& key);

	void allocate(unsigned int newSize);

	/*
    || @description
    || | Get the index of a key
    || #
    ||
    || @parameter key the key to get the index for
    ||
    || @return The index of the key, or -1 if key does not exist
    */
	int indexOf(const K& key) const;

	/*
    || @description
    || | Check if a key is contained within this HashMap
    || #
    ||
    || @parameter key the key to check if is contained within this HashMap
    ||
    || @return true if it is contained in this HashMap
    */
	bool contains(const K& key) const
	{
		return indexOf(key) >= 0;
	}

	/*
     || @description
     || | Remove entry at given index
     || #
     ||
     || @parameter index location to remove from this HashMap
     */
	void removeAt(unsigned index);

	/*
    || @description
    || | Remove a key from this HashMap
    || #
    ||
    || @parameter key the key to remove from this HashMap
    */
	void remove(const K& key)
	{
		int index = indexOf(key);
		if(index >= 0) {
			removeAt(index);
		}
	}

	void clear();

	void setMultiple(const HashMap<K, V>& map);

	void setNullValue(const V& nullv)
	{
		nil = nullv;
	}

protected:
	K** keys = nullptr;
	V** values = nullptr;
	V nil;
	uint16_t currentIndex = 0;
	uint16_t size = 0;
	comparator cb_comparator = nullptr;

private:
	HashMap(const HashMap<K, V>& that);
};

template <typename K, typename V> V& HashMap<K, V>::operator[](const K& key)
{
	int i = indexOf(key);
	if(i >= 0) {
		return *values[i];
	}
	if(currentIndex >= size) {
		allocate(currentIndex + 1);
	}
	*keys[currentIndex] = key;
	*values[currentIndex] = nil;
	currentIndex++;
	return *values[currentIndex - 1];
}

template <typename K, typename V> void HashMap<K, V>::allocate(unsigned int newSize)
{
	if(newSize <= size)
		return;

	K** nkeys = new K*[newSize];
	V** nvalues = new V*[newSize];

	if(keys != nullptr) {
		for(unsigned i = 0; i < size; i++) {
			nkeys[i] = keys[i];
			nvalues[i] = values[i];
		}

		delete[] keys;
		delete[] values;
	}
	for(unsigned i = size; i < newSize; i++) {
		nkeys[i] = new K();
		nvalues[i] = new V();
	}

	keys = nkeys;
	values = nvalues;
	size = newSize;
}

template <typename K, typename V> int HashMap<K, V>::indexOf(const K& key) const
{
	for(unsigned i = 0; i < currentIndex; i++) {
		if(cb_comparator) {
			if(cb_comparator(key, *keys[i])) {
				return i;
			}
		} else {
			if(key == *keys[i]) {
				return i;
			}
		}
	}
	return -1;
}

template <typename K, typename V> void HashMap<K, V>::removeAt(unsigned index)
{
	if(index >= currentIndex)
		return;

	for(unsigned i = index + 1; i < size; i++) {
		*keys[i - 1] = *keys[i];
		*values[i - 1] = *values[i];
	}

	currentIndex--;
}

template <typename K, typename V> void HashMap<K, V>::clear()
{
	if(keys != nullptr) {
		for(unsigned i = 0; i < size; i++) {
			delete keys[i];
			delete values[i];
		}
		delete[] keys;
		delete[] values;
		keys = nullptr;
		values = nullptr;
	}
	currentIndex = 0;
	size = 0;
}

template <typename K, typename V> void HashMap<K, V>::setMultiple(const HashMap<K, V>& map)
{
	for(unsigned i = 0; i < map.count(); i++) {
		(*this)[map.keyAt(i)] = *(map.values)[i];
	}
}
