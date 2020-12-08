/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Object.h - Base Storage object definition
 *
 ****/
#pragma once

#include "Object.h"
#include <vector>

namespace Storage
{
/**
 * @brief Singly-linked list of objects
 * @note We don't own the items, just keep references to them
 */
class ObjectList
{
public:
	ObjectList()
	{
	}

	ObjectList(Object* object) : mHead(object)
	{
	}

	bool add(Object* object);

	bool add(const Object* object)
	{
		return add(const_cast<Object*>(object));
	}

	bool remove(Object* object);

	void clear()
	{
		mHead = nullptr;
	}

	Object* head()
	{
		return mHead;
	}

	const Object* head() const
	{
		return mHead;
	}

	bool isEmpty() const
	{
		return mHead == nullptr;
	}

protected:
	Object* mHead{nullptr};
};

template <typename ObjectType> class ObjectListTemplate : public ObjectList
{
public:
	ObjectListTemplate() = default;

	ObjectListTemplate(ObjectType* object) : ObjectList(object)
	{
	}

	ObjectType* head()
	{
		return reinterpret_cast<ObjectType*>(mHead);
	}

	const ObjectType* head() const
	{
		return reinterpret_cast<const ObjectType*>(mHead);
	}

	typename ObjectType::Iterator begin()
	{
		return head();
	}

	typename ObjectType::Iterator end()
	{
		return nullptr;
	}

	typename ObjectType::ConstIterator begin() const
	{
		return head();
	}

	typename ObjectType::ConstIterator end() const
	{
		return nullptr;
	}

	size_t count() const
	{
		return std::count(begin(), end(), true);
	}

	bool contains(const ObjectType& object) const
	{
		return std::find(begin(), end(), object);
	}
};

/**
 * @brief Class template for singly-linked list of objects
 * @note We own the objects so are responsible for destroying them when removed
 */
template <typename ObjectType> class OwnedObjectListTemplate : public ObjectListTemplate<ObjectType>
{
public:
	bool remove(ObjectType* object)
	{
		bool res = ObjectList::remove(object);
		delete object;
		return res;
	}

	void clear()
	{
		while(remove(this->head())) {
			//
		}
	}
};

} // namespace Storage
