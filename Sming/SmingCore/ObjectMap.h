/*
 * ObjectMap.h
 *
 *  Created on: 31 Jul 2018
 *      Author: mikee47
 *
 * Implementation of a HashMap for owned objects, i.e. anything created with new()
 *
 * Once added to the map the object is destroyed when no longer required.
 *
 * This class replaces previous usage of HashMap which complicated code
 * and created opportunities for memory leaks.
 *
 */

#ifndef __OBJECTMAP_H
#define __OBJECTMAP_H

#include "WHashMap.h"

/** @brief Maintains a map of object pointers and deletes them as appropriate.
 * 	@param _cls type this map works with (can also be a structure)
 * 	@note This is a map of <Sting, _cls*> and objects are 'owned' by this map, so
 * 	      when removed they are first destroyed (using delete operator).
 */
template <typename _cls> class ObjectMap {
public:
	ObjectMap()
	{
		// Ensures new values are created initialised
		_map.setNullValue(nullptr);
	}

	~ObjectMap()
	{
		clear();
	}

	void set(const String& key, _cls* value)
	{
		_cls*& cur = _map[key];
		delete cur;
		cur = value;
	}

	/*
	 * Return the object if it exists, otherwise nullptr
	 */
	_cls* find(const String& key)
	{
		int index = _map.indexOf(key);
		return (index >= 0) ? _map.valueAt(index) : nullptr;
	}

	/*
	 * Return reference to object pointer. Caller should check for nullptr values and create object as required.
	 *
	 * WARNING: If returned object is deleted, set it to nullptr to prevent problems. Preferably use remove() method.
	 */
	_cls*& operator[](const String& key)
	{
		return _map[key];
	}

	void remove(const String& key)
	{
		int index = _map.indexOf(key);
		if (index >= 0) {
			delete _map.valueAt(index);
			_map.remove(index);
		}
	}

	void clear()
	{
		for (unsigned i = 0; i < _map.count(); ++i) {
			auto& obj = _map.valueAt(i);
			delete obj;
			obj = nullptr;
		}
		_map.clear();
	}

	unsigned count()
	{
		return _map.count();
	}

	/** @brief Extract key and object at given position
	 *  @retval _cls* object or nullptr if index is invalid
	 *  @note  Ownership of object passes to caller
	 */
	_cls* extract(unsigned index, String& key)
	{
		if (index >= _map.count())
			return nullptr;
		key = _map.keyAt(index);
		_cls* value = _map.valueAt(index);
		_map.removeAt(index);
		return value;
	}

private:
	HashMap<String, _cls*> _map;
};

#endif // __OBJECTMAP_H
