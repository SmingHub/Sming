/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ObjectMap.h
 *
 * @author: 31 Jul 2018 - Mikee47 <mike@sillyhouse.net>
 *
 */

#pragma once

#include "WVector.h"

/**
 * @brief Implementation of a HashMap for owned objects, i.e. anything created with new().
 * @note Once added to the map the object is destroyed when no longer required.
 *
 *  Example:
 *
 *  ```
 *	void test()
 *	{
 *		ObjectMap<String, MyType> map;
 *		MyType* object1 = new MyType();
 *		if (map["key1"] == nullptr) { 	// Does NOT create entry in map
 *			map["key1"] = object1; 		// Entry now created, "key1" -> object1
 *		}
 *		MyType* object2 = new MyType();
 *		map["key1"] = object2;			// object1 is destroyed, "key1" -> object2
 *
 *		// Demonstrate use of value reference
 *		auto value = map["key1"];		// Returns ObjectMap<String, MyType>::Value object
 *		value = new MyType();			// "key1" -> new object
 *		value = nullptr; 				// Free object, "key1" -> nullptr (but still in map)
 *		value.remove();					// Free object1 and remove from map
 *
 *		// As soon as `map` goes out of scope, all contained objects are destroyed
 *		map["key1"] = new MyType();
 *		map["key2"] = new MyType();
 *	}
 * 	```
 *
 */
template <typename K, typename V> class ObjectMap
{
public:
	ObjectMap()
	{
	}

	~ObjectMap()
	{
		clear();
	}

	/**
	 * @brief Class to provide safe access to mapped value
	 * @note ObjectMap `operator[]` returns one of these, which provides behaviour consistent with V*
	 */
	class Value
	{
	public:
		Value(ObjectMap<K, V>& map, const K& key) : map(map), key(key)
		{
		}

		const K& getKey() const
		{
			return key;
		}

		V* getValue() const
		{
			return map.find(key);
		}

		Value& operator=(V* newValue)
		{
			map.set(key, newValue);
			return *this;
		}

		operator V*() const
		{
			return getValue();
		}

		V* operator->() const
		{
			return getValue();
		}

		/**
		 * @brief Remove this value from the map
		 * @retval bool true if the value was found and removed
		 */
		bool remove()
		{
			return map.remove(key);
		}

		/**
		 * @brief Get the value for a given key and remove it from the map, without destroying it
		 * @retval V*
		 * @note The returned object must be freed by the caller when no longer required
		 */
		V* extract()
		{
			return map.extract(key);
		}

	private:
		ObjectMap<K, V>& map;
		K key;
	};

	/**
	 * @brief Get the number of entries in this map
	 * @retval int Entry count
	 */
	unsigned count() const
	{
		return entries.count();
	}

	/*
	 * @brief Get a key at a specified index, non-modifiable
	 * @param idx the index to get the key at
	 * @return The key at index idx
	 */
	const K& keyAt(unsigned idx) const
	{
		return entries[idx].key;
	}

	/*
	 * @brief Get a key at a specified index
	 * @param idx the index to get the key at
	 * @return Reference to the key at index idx
	 */
	K& keyAt(unsigned idx)
	{
		return entries[idx].key;
	}

	/*
	 * @brief Get a value at a specified index, non-modifiable
	 * @param idx the index to get the value at
	 * @retval The value at index idx
	 * @note The caller must not use `delete` on the returned value
	 */
	const V* valueAt(unsigned idx) const
	{
		return entries[idx].value;
	}

	/*
	 * @brief Get a value at a specified index
	 * @param idx the index to get the value at
	 * @retval Value Reference to value at index idx
	 * @see `operator[]`
	 */
	Value valueAt(unsigned idx)
	{
		return Value(*this, entries[idx].key);
	}

	/**
	 * @brief Get value for given key, if it exists
	 * @param key
	 * @retval const V* Will be null if not found in the map
	 * @note The caller must not use `delete` on the returned value
	 */
	const V* operator[](const K& key) const
	{
		return find(key);
	}

	/** @brief Access map entry by reference
	 *  @param key
	 *  @retval Value Guarded access to mapped value corresponding to given key
	 *  @note If the given key does not exist in the map it will NOT be created
	 * 	@see `valueAt()`
	 *
	 */
	Value operator[](const K& key)
	{
		return get(key);
	}

	/** @brief Get map entry value
	 *  @param key
	 *  @retval Value
	 *  @see `operator[]`
	 */
	Value get(const K& key)
	{
		return Value(*this, key);
	}

	/** @brief Set a key value
	 *  @param key
	 *  @param value
	 */
	void set(const K& key, V* value)
	{
		int i = indexOf(key);
		if(i >= 0) {
			delete entries[i].value;
			entries[i].value = value;
		} else {
			entries.addElement(new Entry(key, value));
		}
	}

	/**
	 * @brief Find the value for a given key, if it exists
	 * @param key
	 * @retval V* Points to the object if it exists, otherwise nullptr
	 * @note If you need to modify the existing map entry, use `operator[]` or `valueAt()`
	 */
	V* find(const K& key) const
	{
		int index = indexOf(key);
		return (index < 0) ? nullptr : entries[index].value;
	}

	/**
	 * @brief Get the index of a key
	 * @param key
	 * @retval int The index of the key, or -1 if key does not exist
	 */
	int indexOf(const K& key) const
	{
		for(unsigned i = 0; i < entries.count(); i++) {
			if(entries[i].key == key) {
				return i;
			}
		}
		return -1;
	}

	/**
	 * @brief Check if a key is contained within this map
	 * @param key the key to check
	 * @retval bool true if key exists
	 */
	bool contains(const K& key) const
	{
		return indexOf(key) >= 0;
	}

	/**
	 * @brief Remove entry at given index
	 * @param index location to remove from this map
	 */
	void removeAt(unsigned index)
	{
		entries.remove(index);
	}

	/**
	 * @brief Remove a key from this map
	 * @param key The key identifying the entry to remove
	 * @retval bool true if the value was found and removed
	 */
	bool remove(const K& key)
	{
		int index = indexOf(key);
		if(index < 0) {
			return false;
		} else {
			removeAt(index);
			return true;
		}
	}

	/**
	 * @brief Get the value for a given key and remove it from the map, without destroying it
	 * @param key
	 * @retval V*
	 * @note The returned object must be freed by the caller when no longer required
	 */
	V* extract(const K& key)
	{
		int i = indexOf(key);
		return (i < 0) ? nullptr : extractAt(i);
	}

	/**
	 * @brief Get the value at a given index and remove it from the map, without destroying it
	 * @param index
	 * @retval V*
	 * @note The returned object must be freed by the caller when no longer required
	 */
	V* extractAt(unsigned index)
	{
		V* value = nullptr;
		if(index < entries.count()) {
			value = entries[index].value;
			entries[index].value = nullptr;
			entries.remove(index);
		}
		return value;
	}

	/**
	 * @brief Clear the map of all entries
	 */
	void clear()
	{
		entries.clear();
	}

protected:
	/**
	 * @brief An entry in the ObjectMap
	 */
	struct Entry {
		K key;
		V* value = nullptr;

		Entry(const K& key, V* value) : key(key), value(value)
		{
		}

		~Entry()
		{
			delete value;
		}
	};

	Vector<Entry> entries;

private:
	// Copy constructor unsafe, so prevent access
	ObjectMap(ObjectMap<K, V>& that);
};
