/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ToneBufferQueue.cpp
 *
 ****/

#include "ToneBufferQueue.h"

void ToneBufferQueue::append(ToneBuffer* buf)
{
	if(head == nullptr) {
		head = buf;
		return;
	}

	auto p = head;
	while(p->next != nullptr) {
		p = p->next;
	}
	p->next = buf;
}

ToneBuffer* ToneBufferQueue::dequeue()
{
	ToneBuffer* buf = head;
	if(head != nullptr) {
		head = head->next;
		buf->next = nullptr;
	}
	return buf;
}
