/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * LinkedObjectList.h
 *
 ****/
#pragma once

#include "LinkedObject.h"

/**
 * @brief Singly-linked list of objects
 * @note We don't own the items, just keep references to them
 */
class LinkedObjectList
{
public:
	LinkedObjectList()
	{
	}

	LinkedObjectList(LinkedObject* object) : mHead(object)
	{
	}

	bool add(LinkedObject* object);

	bool add(const LinkedObject* object)
	{
		return add(const_cast<LinkedObject*>(object));
	}

	bool remove(LinkedObject* object);

	void clear()
	{
		mHead = nullptr;
	}

	LinkedObject* head()
	{
		return mHead;
	}

	const LinkedObject* head() const
	{
		return mHead;
	}

	bool isEmpty() const
	{
		return mHead == nullptr;
	}

protected:
	LinkedObject* mHead{nullptr};
};

template <typename ObjectType> class LinkedObjectListTemplate : public LinkedObjectList
{
public:
	LinkedObjectListTemplate() = default;

	LinkedObjectListTemplate(ObjectType* object) : LinkedObjectList(object)
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
template <typename ObjectType> class OwnedLinkedObjectListTemplate : public LinkedObjectListTemplate<ObjectType>
{
public:
	bool remove(ObjectType* object)
	{
		bool res = LinkedObjectList::remove(object);
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
