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

#ifndef HASHMAP_H
#define HASHMAP_H

#include "Countable.h"
#include "WiringFrameworkDependencies.h"

template <typename K, typename V> class HashMap {
public:
	typedef bool (*comparator)(const K&, const K&);

	/*
	 || @constructor
	 || | Initialize this HashMap
	 || #
	 ||
	 || @parameter compare optional function for comparing a key against another (for complex types)
	 */
	HashMap(comparator compare = 0)
	{
		_cb_comparator = compare;
		_currentIndex = 0;
		_size = 0;
		_keys = nullptr;
		_values = nullptr;
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
	unsigned count() const
	{
		return _currentIndex;
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
	const K& keyAt(unsigned idx) const
	{
		return *_keys[idx];
	}

	K& keyAt(unsigned idx)
	{
		return *_keys[idx];
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
	const V& valueAt(unsigned idx) const
	{
		return *_values[idx];
	}

	V& valueAt(unsigned idx)
	{
		return *_values[idx];
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
	 || @return The const value for key
	 */
	const V& operator[](const K& key) const
	{
		// Don't create non-existent values
		auto i = indexOf(key);
		return (i >= 0) ? valueAt(i) : _nil;
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
	V& operator[](const K& key)
	{
		int i = indexOf(key);
		if (i >= 0)
			return *_values[i];

		if (_currentIndex >= _size)
			allocate(_currentIndex + 1);
		*_keys[_currentIndex] = key;
		*_values[_currentIndex] = _nil;
		_currentIndex++;
		return *_values[_currentIndex - 1];
	}

	void allocate(unsigned newSize)
	{
		if (newSize <= _size)
			return;

		K** nkeys = new K*[newSize];
		V** nvalues = new V*[newSize];

		if (_keys) {
			for (unsigned i = 0; i < _size; i++) {
				nkeys[i] = _keys[i];
				nvalues[i] = _values[i];
			}

			delete[] _keys;
			delete[] _values;
		}
		for (unsigned i = _size; i < newSize; i++) {
			nkeys[i] = new K();
			nvalues[i] = new V();
		}

		_keys = nkeys;
		_values = nvalues;
		_size = newSize;
	}

	/*
	 || @description
	 || | Get the index of a key
	 || #
	 ||
	 || @parameter key the key to get the index for
	 ||
	 || @return The index of the key, or -1 if key does not exist
	 */
	int indexOf(const K& key) const
	{
		for (unsigned i = 0; i < _currentIndex; i++) {
			if (_cb_comparator) {
				if (_cb_comparator(key, *_keys[i]))
					return i;
			}
			else if (key == *_keys[i])
				return i;
		}

		return -1;
	}

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
	void removeAt(unsigned index)
	{
		if (index >= _currentIndex)
			return;

		for (unsigned i = index + 1; i < _size; i++) {
			*_keys[i - 1] = *_keys[i];
			*_values[i - 1] = *_values[i];
		}

		_currentIndex--;
	}

	/*
	 || @description
	 || | Check if a key is contained within this HashMap
	 || #
	 ||
	 || @parameter key the key to remove from this HashMap
	 */
	void remove(const K& key)
	{
		int index = indexOf(key);
		if (index >= 0)
			removeAt(index);
	}

	void clear()
	{
		if (_keys) {
			for (unsigned i = 0; i < _size; i++) {
				delete _keys[i];
				delete _values[i];
			}
			delete[] _keys;
			delete[] _values;
			_keys = NULL;
			_values = NULL;
		}
		_currentIndex = 0;
		_size = 0;
	}

	void setMultiple(const HashMap<K, V>& map)
	{
		for (unsigned i = 0; i < map.count(); i++)
			(*this)[map.keyAt(i)] = map.valueAt(i);
	}

	void setNullValue(const V& nullv)
	{
		_nil = nullv;
	}

protected:
	K** _keys;
	V** _values;
	V _nil;
	uint16_t _currentIndex;
	uint16_t _size;
	comparator _cb_comparator;

private:
	HashMap(const HashMap<K, V>& that);
};

#endif
// HASHMAP_H
