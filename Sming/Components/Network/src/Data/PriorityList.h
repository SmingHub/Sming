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

template <typename ObjectType> class PriorityNode
{
public:
	ObjectType data;
	int priority;
	PriorityNode* next;
};

template <typename ObjectType> class PriorityList
{
public:
	~PriorityList()
	{
		auto current = head;
		while(current != nullptr) {
			auto next = current->next;
			delete current;
			current = next;
		}
	}

	/**
	 * @brief Adds and element and orders it according to its priority. Order is: High to low.
	 * @param prioty
	 *
	 * @retval bool true on success
	 */
	bool add(ObjectType object, int priority)
	{
		auto node = new PriorityNode<ObjectType>{};
		if(node == nullptr) {
			return false;
		}

		node->data = object;
		node->priority = priority;

		if(head == nullptr) {
			head = node;
			return true;
		}

		auto current = head;
		if(current->priority < node->priority) {
			node->next = current;
			head = node;
			return true;
		}

		while(current->next != nullptr && current->next->priority > node->priority) {
			current = current->next;
		}

		node->next = current->next;
		current->next = node;

		return true;
	}

	/**
	 * @brief Gets the head of the ordered linked list
	 * @retval pointer
	 */
	PriorityNode<ObjectType>* getHead() const
	{
		return head;
	}

private:
	PriorityNode<ObjectType>* head = nullptr;
};
