#pragma once

#include "Message.h"

namespace mDNS
{
class Request : public Message
{
public:
	Request(Type type);

	Question* addQuestion(const String& name, ResourceType type = ResourceType::PTR, uint16_t qclass = 1,
						  bool unicast = false);

	Answer* createAnswer(const String& name, Resource::Type type, uint16_t rclass = 1, bool flush = false,
						 uint32_t ttl = 120);

	template <class Resource, typename... ParamTypes> Resource addAnswer(const String& name, ParamTypes... params)
	{
		auto answer = createAnswer(name, Resource::type);
		Resource r(*answer);
		r.init(params...);
		return r;
	}

	Answer::Kind nextSection()
	{
		assert(kind < Answer::Kind::additional);
		kind = Answer::Kind(unsigned(kind) + 1);
		return kind;
	}

private:
	uint8_t buffer[1024];
	Answer::Kind kind{Answer::Kind::answer};
};

} // namespace mDNS
