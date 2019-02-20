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
 * Implementation of a HashMap for owned objects, i.e. anything created with new().
 * Once added to the map the object is destroyed when no longer required.
 *
 */

#ifndef _SMING_CORE_DATA_OBJECT_MAP_H_
#define _SMING_CORE_DATA_OBJECT_MAP_H_

#include "WVector.h"

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
	 * @retval The value at index idx
	 * @note Because a reference is returned any existing value must be `delete`d first
	 * @see `operator[]`
	 * @see `set()`
	 */
	V*& valueAt(unsigned idx)
	{
		return entries[idx].value;
	}

	/**
	 * @brief Get value for given key, if it exists
	 * @param key
	 * @retval const V* Will be null if not found in the map
	 */
	const V* operator[](const K& key) const
	{
		return find(key);
	}

	/** @brief Access map entry by reference
	 *  @param key
	 *  @retval V*& Reference to mapped value corresponding to given key
	 *  @note If the given key does not exist in the map then it will be created
	 *  and a null value entry returned.
	 *  Be careful to check for existing value before assigning to avoid memory leaks.
	 *  In most cases `set()` should be used to avoid this complication.
	 *
	 *  Example:
	 *
	 *  ```
	 *	void test()
	 *	{
	 *		ObjectMap<String, MyType> map;
	 *		MyType* object1 = new MyType();
	 *		MyType* object2 = new MyType();
	 *		map["key1"] = object1;
	 *		auto& value = map["key1"]; // value now refers to object1
	 *		delete value;
	 *		value = object2;
	 *		// As soon as `map` goes out of scope, object2 is released
	 *	}
	 * 	```
	 *
	 * 	@see `set()`
	 * 	@see `valueAt()`
	 *
	 */
	V*& operator[](const K& key)
	{
		int i = indexOf(key);
		if(i >= 0) {
			return entries[i].value;
		}

		auto entry = new Entry(key, nullptr);
		entries.addElement(entry);
		return entry->value;
	}

	/**
	 * @brief Set a key value, ensuring any existing value is released
	 *  @param key
	 *  @param value
	 *  @note Example:
	 *  Example:
	 *
	 *		void test()
	 *		{
	 *			ObjectMap<String, MyType> map;
	 *			MyType* object1 = new MyType();
	 *			MyType* object2 = new MyType();
	 *			map.set("key1", object1);
	 *			map.set("key1", object2); // object1 is freed automatically
	 *			// As soon as `map` goes out of scope, object2 is released
	 *		}
	 *
	 */
	void set(const K& key, V* value)
	{
		auto& cur = operator[](key);
		delete cur;
		cur = value;
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
	 */
	void remove(const K& key)
	{
		int index = indexOf(key);
		if(index >= 0) {
			removeAt(index);
		}
	}

	/**
	 * @brief Clear the map of all entries
	 */
	void clear()
	{
		entries.clear();
	}

protected:
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
	ObjectMap(const ObjectMap<K, V>& that);
};

#endif // _SMING_CORE_DATA_OBJECT_MAP_H_
