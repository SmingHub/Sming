/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 *
 * @author: 2021 - Slavey Karadzhov <slaff@attachix.com>
 *
 ****/
#pragma once

#include "LinkedObjectList.h"

template <typename ObjectType> class PriorityNode : public LinkedObjectTemplate<PriorityNode<ObjectType>>
{
public:
	PriorityNode(ObjectType data, int priority) : data(data), priority(priority)
	{
	}

	ObjectType data;
	int priority;
};

template <typename ObjectType> class PriorityList : public OwnedLinkedObjectListTemplate<PriorityNode<ObjectType>>
{
public:
	using List = OwnedLinkedObjectListTemplate<PriorityNode<ObjectType>>;

	/**
	 * @brief Adds and element and orders it according to its priority. Order is: High to low.
	 * @param object
	 * @param priority
	 *
	 * @retval bool true on success
	 */
	bool add(ObjectType object, int priority)
	{
		auto node = new PriorityNode<ObjectType>{object, priority};
		if(node == nullptr) {
			return false;
		}

		if(List::isEmpty()) {
			List::add(node);
			return true;
		}

		auto current = this->head();
		if(current->priority < node->priority) {
			List::insert(node);
			return true;
		}

		while(current->next() != nullptr && current->getNext()->priority > node->priority) {
			current = current->getNext();
		}

		node->insertAfter(current);

		return true;
	}
};
