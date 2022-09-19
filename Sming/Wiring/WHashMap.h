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
#include <iterator>
#include <cstdlib>
#include "WiringList.h"

/**
 * @brief HashMap class template
 * @ingroup wiring
 */
template <typename K, typename V> class HashMap
{
public:
	using Comparator = bool (*)(const K&, const K&);

	template <bool is_const> struct BaseElement {
	public:
		using Value = typename std::conditional<is_const, const V, V>::type;

		BaseElement(const K& key, Value& value) : k(key), v(value)
		{
		}

		const K& key() const
		{
			return k;
		}

		Value& value()
		{
			return v;
		}

		const V& value() const
		{
			return v;
		}

		BaseElement& operator=(const V& value)
		{
			v = value;
			return *this;
		}

		Value& operator*()
		{
			return v;
		}

		const Value& operator*() const
		{
			return v;
		}

		Value* operator->()
		{
			return &v;
		}

		const Value* operator->() const
		{
			return &v;
		}

	private:
		const K& k;
		Value& v;
	};

	using Element = BaseElement<false>;
	using ElementConst = BaseElement<true>;

	template <bool is_const>
	class Iterator : public std::iterator<std::random_access_iterator_tag, BaseElement<is_const>>
	{
	public:
		using Map = typename std::conditional<is_const, const HashMap, HashMap>::type;
		using Value = typename std::conditional<is_const, const V, V>::type;

		Iterator(const Iterator&) = default;

		Iterator(Map& map, unsigned index) : map(map), index(index)
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
			return &map == &rhs.map && index == rhs.index;
		}

		bool operator!=(const Iterator& rhs) const
		{
			return !operator==(rhs);
		}

		BaseElement<is_const> operator*()
		{
			return BaseElement<is_const>{map.keyAt(index), map.valueAt(index)};
		}

		ElementConst operator*() const
		{
			return ElementConst{map.keyAt(index), map.valueAt(index)};
		}

	private:
		Map& map;
		unsigned index{0};
	};

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
	HashMap(Comparator compare) : cb_comparator(compare)
	{
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
		return keys[idx];
	}

	K& keyAt(unsigned int idx)
	{
		if(idx >= count()) {
			abort();
		}
		return keys[idx];
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
		return values[idx];
	}

	V& valueAt(unsigned int idx)
	{
		if(idx >= count()) {
			abort();
		}
		return values[idx];
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
		return (i >= 0) ? values[i] : nil;
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

	bool allocate(unsigned int newSize);

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

	Iterator<false> begin()
	{
		return Iterator<false>(*this, 0);
	}

	Iterator<false> end()
	{
		return Iterator<false>(*this, count());
	}

	Iterator<true> begin() const
	{
		return Iterator<true>(*this, 0);
	}

	Iterator<true> end() const
	{
		return Iterator<true>(*this, count());
	}

protected:
	using KeyList = wiring_private::List<K>;
	using ValueList = wiring_private::List<V>;

	KeyList keys;
	ValueList values;
	Comparator cb_comparator{nullptr};
	unsigned currentIndex{0};
	V nil{};

private:
	HashMap(const HashMap<K, V>& that);
};

template <typename K, typename V> V& HashMap<K, V>::operator[](const K& key)
{
	int i = indexOf(key);
	if(i >= 0) {
		return values[i];
	}
	if(currentIndex >= values.size) {
		allocate(currentIndex + ((values.size < 16) ? 4 : 16));
	}
	keys[currentIndex] = key;
	values[currentIndex] = nil;
	currentIndex++;
	return values[currentIndex - 1];
}

template <typename K, typename V> bool HashMap<K, V>::allocate(unsigned int newSize)
{
	return keys.allocate(newSize, K{}) && values.allocate(newSize, nil);
}

template <typename K, typename V> int HashMap<K, V>::indexOf(const K& key) const
{
	for(unsigned i = 0; i < currentIndex; i++) {
		if(cb_comparator) {
			if(cb_comparator(key, keys[i])) {
				return i;
			}
		} else if(key == keys[i]) {
			return i;
		}
	}
	return -1;
}

template <typename K, typename V> void HashMap<K, V>::removeAt(unsigned index)
{
	if(index >= currentIndex) {
		return;
	}

	keys.remove(index);
	values.remove(index);

	currentIndex--;
}

template <typename K, typename V> void HashMap<K, V>::clear()
{
	keys.clear();
	values.clear();
	currentIndex = 0;
}

template <typename K, typename V> void HashMap<K, V>::setMultiple(const HashMap<K, V>& map)
{
	for(auto e : map) {
		(*this)[e.key()] = e.value();
	}
}
