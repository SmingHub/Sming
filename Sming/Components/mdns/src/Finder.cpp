#include <Network/Mdns/Finder.h>

String toString(mDNS::ResourceType type)
{
	switch(type) {
#define XX(name, value, desc)                                                                                          \
	case mDNS::ResourceType::name:                                                                                     \
		return F(#name);
		MDNS_RESOURCE_TYPE_MAP(XX)
#undef XX
	default:
		return String(unsigned(type));
	}
}

namespace
{
struct Packet {
	uint8_t* data;
	mutable uint16_t pos;

	void* ptr() const
	{
		return data + pos;
	}

	uint8_t peek8() const
	{
		return data[pos];
	}

	uint8_t read8() const
	{
		return data[pos++];
	}

	uint16_t read16() const
	{
		return (read8() << 8) | read8();
	}

	uint32_t read32() const
	{
		return (read16() << 16) | read16();
	}

	void write(const void* s, uint16_t len)
	{
		memcpy(&data[pos], s, len);
		pos += len;
	}

	void write8(uint8_t value)
	{
		data[pos++] = value;
	}

	void write16(uint16_t value)
	{
		write8(value >> 8);
		write8(value & 0xff);
	}
};

bool writeToBuffer(uint8_t value, char* name, uint16_t& namePos, uint16_t nameLength)
{
	if(namePos + 1 < nameLength) {
		name[namePos++] = value;
		name[namePos] = '\0';
		return true;
	}

	namePos++;
	return false;
}

void parseText(Packet& pkt, char* buffer, uint16_t bufferLength, uint16_t dataLength)
{
	uint16_t bufferPos{0};
	for(uint16_t i = 0; i < dataLength; i++) {
		writeToBuffer(pkt.read8(), buffer, bufferPos, bufferLength);
	}
	buffer[bufferPos] = '\0';
}

void nameFromDnsPointer(const Packet& pkt, char* name, uint16_t namePos, uint16_t nameLength, bool recurse)
{
	if(recurse) {
		// Since we are adding more to an already populated buffer,
		// replace the trailing EOL with the FQDN seperator.
		namePos--;
		writeToBuffer('.', name, namePos, nameLength);
	}

	if(pkt.peek8() < 0xC0) {
		// Since the first 2 bits are not set,
		// this is the start of a name section.
		// http://www.tcpipguide.com/free/t_DNSNameNotationandMessageCompressionTechnique.htm

		const uint8_t word_len = pkt.read8();
		for(uint8_t l = 0; l < word_len; l++) {
			writeToBuffer(pkt.read8(), name, namePos, nameLength);
		}

		writeToBuffer('\0', name, namePos, nameLength);

		if(pkt.peek8() > 0) {
			// Next word
			nameFromDnsPointer(pkt, name, namePos, nameLength, true);
		} else {
			// End of string.
			pkt.read8();
		}
	} else {
		// Message Compression used. Next 2 bytes are a pointer to the actual name section.
		uint16_t pointer = pkt.read16() & 0x3000;
		Packet tmp{pkt.data, pointer};
		nameFromDnsPointer(tmp, name, namePos, nameLength, false);
	}
}

} // namespace

namespace mDNS
{
Finder::~Finder()
{
	if(initialised) {
		UdpConnection::leaveMulticastGroup(IpAddress(MDNS_IP));
	}
}

bool Finder::search(const String& hostname, ResourceType type)
{
	if(hostname.length() > MAX_MDNS_NAME_LEN - 1) {
		return false;
	}

	Query query{};
	memcpy(query.name, hostname.c_str(), hostname.length());
	query.type = type;
	query.klass = 1; // "INternet"
	query.isUnicastResponse = false;
	query.isValid = true;

	return search(query);
}

bool Finder::search(const Query& query)
{
	uint8_t buffer[MAX_PACKET_SIZE]{};

	Packet pkt{buffer, 0};

	// The first two bytes are the transaction id and they are not used in MDNS
	pkt.write16(0);

	// 2 bytes for Flags
	pkt.write8(0); // 0b00000000 for Query, 0b10000000 for Answer.
	pkt.write8(0);

	// 2 bytes for number of questions
	pkt.write16(1);

	// 2 bytes for number of Answer RRs
	pkt.write16(0);

	// 2 bytes for Authority PRs
	pkt.write16(0);

	// 2 bytes for Additional PRs
	pkt.write16(0);

	uint16_t word_start{0};
	uint16_t word_end{0};

	const char* name = query.name;
	while(true) {
		if(name[word_end] == '.' || name[word_end] == '\0') {
			const uint8_t word_length = word_end - word_start;

			pkt.write8(word_length);
			pkt.write(&name[word_start], word_length);
			if(name[word_end] == '\0') {
				break;
			}
			word_end++; // Skip the '.' character.
			word_start = word_end;
		}
		word_end++;
	}

	pkt.write8('\0'); // End of name.

	// 2 bytes for type
	pkt.write16(uint16_t(query.type));

	// 2 bytes for class
	unsigned int qclass = 0;
	if(query.isUnicastResponse) {
		qclass = 0b1000000000000000;
	}
	qclass += query.klass;
	pkt.write16(qclass);

	initialise();
	listen(0);
	return sendTo(IpAddress(MDNS_IP), MDNS_TARGET_PORT, reinterpret_cast<const char*>(pkt.data), pkt.pos);
}

void Finder::initialise()
{
	if(!initialised) {
		joinMulticastGroup(IpAddress(MDNS_IP));
		listen(MDNS_SOURCE_PORT);
		setMulticastTtl(MDNS_TTL);
		initialised = true;
	}
}

void Finder::UdpOut::onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort)
{
	finder.onReceive(buf, remoteIP, remotePort);
}

void Finder::onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort)
{
	if(!answerCallback) {
		return;
	}

	// process the answer here...
	auto data = static_cast<uint8_t*>(buf->payload);
	auto size = buf->len;

	// m_printHex("MDNS", data, size, 0);

	// check if we have a response or a query
	if(!(data[2] & 0b10000000)) {
		debug_d("Message is not a response. Ignoring.");
		return;
	}

	bool truncated = data[2] & 0b00000010;
	// TODO: If it's truncated we can expect more data soon so we should wait for additional records before deciding whether to respond.

	if(data[3] & 0b00001111) {
		// Non zero Response code implies error.
		debug_w("Got errored MDNS answer");
		return;
	}

	Packet pkt{data, 4};

	// Number of incoming queries.
	uint16_t questionsCount = pkt.read16();
	if(questionsCount > 0) {
		// we are interested only in responses.
		return;
	}

	// Number of incoming answers.
	uint16_t answersCount = pkt.read16();

	// Number of incoming Name Server resource records.
	uint16_t nsCount = pkt.read16();

	// Number of incoming Additional resource records.
	uint16_t additionalCount = pkt.read16();

	// List of answers
	for(uint16_t i = 0; i < (answersCount + nsCount + additionalCount); i++) {
		Answer answer{};

		nameFromDnsPointer(pkt, answer.name, 0, MAX_MDNS_NAME_LEN, false);

		answer.type = ResourceType(pkt.read16());

		uint8_t rrclass_0 = pkt.read8();
		uint8_t rrclass_1 = pkt.read8();
		answer.isCachedFlush = (0b10000000 & rrclass_0);
		answer.klass = ((rrclass_0 & 0b01111111) << 8) + rrclass_1;

		answer.ttl = pkt.read32();

		if(pkt.pos > size_t(size)) {
			debug_e("[MDNS] Packet overrun, pos = %u, size = %u", pkt.pos, size);
			// Something has gone wrong receiving or parsing the data.
			answer.isValid = false;
			if(answerCallback) {
				answerCallback(answer);
			}
			break;
		}

		uint16_t rdlength = pkt.read16();

		answer.rawData = pkt.ptr();
		answer.rawDataLen = rdlength;

		switch(answer.type) {
		case ResourceType::A: // Returns a 32-bit IPv4 address
			answer.dataLen = sprintf(answer.data, "%u.%u.%u.%u", pkt.read8(), pkt.read8(), pkt.read8(), pkt.read8());
			break;

		case ResourceType::PTR: // Pointer to a canonical name.
			nameFromDnsPointer(pkt, answer.data, 0, MAX_MDNS_NAME_LEN, false);
			answer.dataLen = strlen(answer.data);
			break;

		case ResourceType::HINFO: // HINFO. host information
			parseText(pkt, answer.data, MAX_MDNS_NAME_LEN, rdlength);
			answer.dataLen = rdlength;
			break;

		case ResourceType::TXT: // Originally for arbitrary human-readable text in a DNS record.
			// We only return the first MAX_MDNS_NAME_LEN bytes of this record type.
			parseText(pkt, answer.data, MAX_MDNS_NAME_LEN, rdlength);
			answer.dataLen = rdlength;
			break;

		case ResourceType::AAAA: { // Returns a 128-bit IPv6 address.
			uint16_t data_pos{0};
			for(uint16_t i = 0; i < rdlength; i++) {
				auto c = pkt.read8();
				if(data_pos < MAX_MDNS_NAME_LEN - 3) {
					sprintf(answer.data + data_pos, "%02X:", c);
				}
				data_pos += 3;
			}
			answer.data[data_pos - 1] = '\0'; // Remove trailing ':'
			answer.dataLen = data_pos;
			break;
		}

		case ResourceType::SRV: { // Server Selection.
			auto priority = pkt.read16();
			auto weight = pkt.read16();
			auto port = pkt.read16();
			auto len = sprintf(answer.data, "p=%u;w=%u;port=%u;host=", priority, weight, port);
			nameFromDnsPointer(pkt, answer.data, len, MAX_MDNS_NAME_LEN - len - 1, false);
			answer.dataLen = strlen(answer.data);
			break;
		}

		default:
			uint16_t data_pos{0};
			for(uint16_t i = 0; i < rdlength; i++) {
				auto c = pkt.read8();
				if(data_pos < MAX_MDNS_NAME_LEN - 3) {
					sprintf(answer.data + data_pos, "%02X ", c);
				}
				data_pos += 3;
			}
			answer.dataLen = strlen(answer.data);
		}

		answer.isValid = true;
		answerCallback(answer);
	}
}

} // namespace mDNS
