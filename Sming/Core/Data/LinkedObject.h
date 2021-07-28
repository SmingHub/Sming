/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * LinkedObject.h
 *
 ****/
#pragma once

#include <iterator>
#include <algorithm>

/**
 * @brief Base virtual class to allow objects to be linked together
 * 
 * This can be more efficient than defining a separate list, as each object
 * requires only an additional pointer field for 'next'.
 */
class LinkedObject
{
public:
	virtual ~LinkedObject()
	{
	}

	virtual LinkedObject* next() const
	{
		return mNext;
	}

	LinkedObject* getNext() const
	{
		return mNext;
	}

	bool insertAfter(LinkedObject* object)
	{
		if(object == nullptr) {
			return false;
		}
		mNext = object->mNext;
		object->mNext = this;
		return true;
	}

	bool operator==(const LinkedObject& other) const
	{
		return this == &other;
	}

	bool operator!=(const LinkedObject& other) const
	{
		return this != &other;
	}

private:
	friend class LinkedObjectList;
	LinkedObject* mNext{nullptr};
};

/**
 * @brief Base class template for linked items with type casting
 */
template <typename ObjectType> class LinkedObjectTemplate : public LinkedObject
{
public:
	template <typename T, typename TPtr, typename TRef>
	class IteratorTemplate : public std::iterator<std::forward_iterator_tag, T>
	{
	public:
		IteratorTemplate(TPtr x) : mObject(x)
		{
		}

		IteratorTemplate(TRef& x) : mObject(&x)
		{
		}

		IteratorTemplate(const IteratorTemplate& other) : mObject(other.mObject)
		{
		}

		IteratorTemplate& operator++()
		{
			mObject = mObject->getNext();
			return *this;
		}

		IteratorTemplate operator++(int)
		{
			Iterator tmp(*this);
			operator++();
			return tmp;
		}

		bool operator==(const IteratorTemplate& rhs) const
		{
			return mObject == rhs.mObject;
		}

		bool operator!=(const IteratorTemplate& rhs) const
		{
			return mObject != rhs.mObject;
		}

		TRef operator*()
		{
			return *mObject;
		}

		TPtr operator->()
		{
			return mObject;
		}

		operator TPtr()
		{
			return mObject;
		}

	private:
		TPtr mObject;
	};

	using Iterator = IteratorTemplate<ObjectType, ObjectType*, ObjectType&>;
	using ConstIterator = IteratorTemplate<const ObjectType, const ObjectType*, const ObjectType&>;

	ObjectType* getNext() const
	{
		return reinterpret_cast<ObjectType*>(this->next());
	}

	bool insertAfter(ObjectType* object)
	{
		return LinkedObject::insertAfter(object);
	}

	Iterator begin() const
	{
		return Iterator(this);
	}

	Iterator end() const
	{
		return Iterator(nullptr);
	}

	Iterator cbegin() const
	{
		return ConstIterator(this);
	}

	Iterator cend() const
	{
		return ConstIterator(nullptr);
	}
};
