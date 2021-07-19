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


template <typename ObjectType>
class PriorityNode
{
public:
	ObjectType data;
	int priority;
	PriorityNode* next;
};


template <typename ObjectType>
class PriorityNodeList
{
public:
	~PriorityNodeList()
	{
		auto start = head;
		while(start !=nullptr) {
			auto temp = start;
			start = start->next;
			delete temp;
		}
	}

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

		auto start = head;
		if(start->priority < node->priority) {
			node->next = start;
			head = node;
			return  true;
		}

		while(start->next != nullptr && start->next->priority > node->priority) {
			start = start->next;
		}

		node->next = start->next;
		start->next = node;

		return true;
	}

	PriorityNode<ObjectType>* getHead() const
	{
		return head;
	}


private:
	PriorityNode<ObjectType>* head = nullptr;
};
