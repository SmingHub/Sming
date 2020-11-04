/**
 * Since big portions of the code are copied from Finder.h and Finder.cpp are copied
 * from MrDunk's mDNS code these files are distributed under the same license as his project.
 *
 * MIT license: https://github.com/mrdunk/esp8266_mdns/blob/master/LICENCE.txt
 */
#pragma once

#include <user_config.h>
#include <Delegate.h>
#include <WString.h>
#include <Network/UdpConnection.h>

namespace mDNS
{
enum QuestionType {
	MDNS_TYPE_A = 0x0001,
	MDNS_TYPE_PTR = 0x000C,
	MDNS_TYPE_HINFO = 0x000D,
	MDNS_TYPE_TXT = 0x0010,
	MDNS_TYPE_AAAA = 0x001C,
	MDNS_TYPE_SRV = 0x0021
};

#define MDNS_IP 224, 0, 0, 251
#define MDNS_TARGET_PORT 5353
#define MDNS_SOURCE_PORT 5353
#define MDNS_TTL 255

#define MAX_PACKET_SIZE 1024

// The mDNS spec says this should never be more than 256 (including trailing '\0').
#define MAX_MDNS_NAME_LEN 256

// A single mDNS Query.
typedef struct Query {
	char name[MAX_MDNS_NAME_LEN]; // Question Name: Contains the object, domain or zone name.
	enum QuestionType type;		  // Question Type: Type of question being asked by client.
	unsigned int klass;			  // Question Class: Normally the value 1 for Internet (“IN”)
	bool isUnicastResponse;		  //
	bool isValid;				  // False if problems were encountered decoding packet.
} Query;

// A single mDNS Answer.
typedef struct Answer {
	char name[MAX_MDNS_NAME_LEN]; // object, domain or zone name.
	char data[MAX_MDNS_NAME_LEN]; // The data portion of the resource record.
	unsigned int type;			  // ResourceRecord Type.
	unsigned int klass;			  // ResourceRecord Class: Normally the value 1 for Internet (“IN”)
	unsigned long int ttl;		  // ResourceRecord Time To Live: Number of seconds ths should be remembered.
	bool isCachedFlush;			  // Flush cache of records matching this name.
	bool isValid;				  // False if problems were encountered decoding packet.
} Answer;

class Finder : protected UdpConnection
{
public:
	using AnswerCallback = Delegate<void(Answer& answer)>;

	Finder();

	~Finder();

	void setAnswerCallback(AnswerCallback callback)
	{
		onAnswer = callback;
	}

	bool search(const String& hostname, const QuestionType& type = MDNS_TYPE_SRV);

	bool search(const Query& query);

protected:
	void onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort) override;

	void processData(UdpConnection& connection, char* data, int size, IpAddress remoteIP, uint16_t remotePort);

private:
	AnswerCallback onAnswer;

	bool writeToBuffer(const uint8_t value, char* p_name_buffer, int* p_name_buffer_pos, const int name_buffer_len);
	int parseText(char* buffer, const int bufferLength, const int dataLength, const uint8_t* packet, int packetPos);

	int nameFromDnsPointer(char* name, int namePos, const int nameLEngth, const uint8_t* packet, int packetPos,
						   const bool recurse = false);
};

} // namespace mDNS
