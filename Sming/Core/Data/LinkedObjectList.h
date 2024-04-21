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

	bool insert(LinkedObject* object)
	{
		if(object == nullptr) {
			return false;
		}

		object->mNext = mHead;
		mHead = object;
		return true;
	}

	bool insert(const LinkedObject* object)
	{
		return insert(const_cast<LinkedObject*>(object));
	}

	bool remove(LinkedObject* object);

	LinkedObject* pop()
	{
		if(mHead == nullptr) {
			return nullptr;
		}
		auto obj = mHead;
		mHead = mHead->mNext;
		obj->mNext = nullptr;
		return obj;
	}

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
	using Iterator =
		typename LinkedObjectTemplate<ObjectType>::template IteratorTemplate<ObjectType, ObjectType*, ObjectType&>;
	using ConstIterator =
		typename LinkedObjectTemplate<ObjectType>::template IteratorTemplate<const ObjectType, const ObjectType*,
																			 const ObjectType&>;

	LinkedObjectListTemplate() = default;

	LinkedObjectListTemplate(ObjectType* object) : LinkedObjectList(object)
	{
	}

	ObjectType* head()
	{
		return static_cast<ObjectType*>(mHead);
	}

	const ObjectType* head() const
	{
		return static_cast<const ObjectType*>(mHead);
	}

	Iterator begin()
	{
		return head();
	}

	Iterator end()
	{
		return nullptr;
	}

	ConstIterator begin() const
	{
		return head();
	}

	ConstIterator end() const
	{
		return nullptr;
	}

	bool add(ObjectType* object)
	{
		return LinkedObjectList::add(object);
	}

	bool add(const ObjectType* object)
	{
		return LinkedObjectList::add(object);
	}

	bool insert(ObjectType* object)
	{
		return LinkedObjectList::insert(object);
	}

	bool insert(const ObjectType* object)
	{
		return LinkedObjectList::insert(object);
	}

	ObjectType* pop()
	{
		return static_cast<ObjectType*>(LinkedObjectList::pop());
	}

	size_t count() const
	{
		return std::count_if(begin(), end(), [](const ObjectType&) { return true; });
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
	OwnedLinkedObjectListTemplate() = default;

	OwnedLinkedObjectListTemplate(const OwnedLinkedObjectListTemplate& other) = delete;
	OwnedLinkedObjectListTemplate& operator=(const OwnedLinkedObjectListTemplate& other) = delete;

	~OwnedLinkedObjectListTemplate()
	{
		clear();
	}

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
