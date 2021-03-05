#include "Network/Mdns/debug.h"

namespace
{
void print(Print& p, const String& tag, const String& value)
{
	String s = "   ";
	s += tag;
	s += ':';
	while(s.length() < 12) {
		s += ' ';
	}
	p.print(s);
	p.print(' ');
	p.println(value);
}

void printHex(Print& p, const String& tag, uint16_t value)
{
	char buf[10];
	m_snprintf(buf, sizeof(buf), "0x%04x", value);
	print(p, tag, buf);
}

void printBool(Print& p, const String& tag, bool value)
{
	print(p, tag, value ? "Y" : "N");
}

} // namespace

namespace mDNS
{
void printQuestion(Print& p, mDNS::Question& question)
{
	p.println(F(">> Question"));
	print(p, F("name"), question.getName());
	auto type = question.getType();
	print(p, F("type"), toString(type));
	printHex(p, F("type"), uint16_t(type));
	printHex(p, F("class"), question.getClass());
	printBool(p, F("unicast"), question.isUnicastReply());
}

void printAnswer(Print& p, mDNS::Answer& answer)
{
	p.print(">> ");
	p.println(toString(answer.getKind()));
	print(p, F("name"), answer.getName());
	print(p, F("data"), answer.getRecordString());
	auto type = answer.getType();
	print(p, F("type"), toString(type));
	printHex(p, F("type"), uint16_t(type));
	print(p, F("ttl"), String(answer.getTtl()));
	printHex(p, F("class"), answer.getClass());
	printBool(p, F("flush"), answer.isCachedFlush());
}

void printMessage(Print& p, mDNS::Message& message)
{
	p.println();
	p.print(system_get_time());
	p.print(' ');
	p.print(message.isReply() ? F("REPLY") : F("QUERY"));
	auto ip = message.getRemoteIp();
	if(uint32_t(ip) != 0) {
		p.print(F(" from "));
		p.print(message.getRemoteIp().toString());
		p.print(':');
		p.println(message.getRemotePort());
	} else {
		p.println();
	}

	p.print(F("Size: "));
	p.print(message.getSize());
	p.println(F(" bytes"));

	for(auto& question : message.questions) {
		printQuestion(p, question);
	}

	for(auto& answer : message.answers) {
		printAnswer(p, answer);
	}
}

} // namespace mDNS
