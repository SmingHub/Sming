/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ToneBufferQueue.h
 *
 * @author Sept 2019 mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include "ToneBuffer.h"
#include <esp_attr.h>

class ToneBufferQueue
{
public:
	~ToneBufferQueue()
	{
		clear();
	}

	void insert(ToneBuffer* buf)
	{
		buf->next = head;
		head = buf;
	}

	void append(ToneBuffer* buf);

	void append(ToneBufferQueue& queue)
	{
		append(queue.head);
		queue.head = nullptr;
	}

	ToneBuffer* IRAM_ATTR dequeue();

	ToneBuffer* peek() const
	{
		return head;
	}

	bool empty() const
	{
		return head == nullptr;
	}

	void clear()
	{
		delete head;
		head = nullptr;
	}

private:
	ToneBuffer* head = nullptr;
};
