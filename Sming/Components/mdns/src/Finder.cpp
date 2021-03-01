#include <Network/Mdns/Finder.h>

namespace mDNS
{
Finder::~Finder()
{
	if(initialised) {
		UdpConnection::leaveMulticastGroup(IpAddress(MDNS_IP));
	}
}

bool Finder::search(const String& hostname, QuestionType type)
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

	uint16_t pos{0};
	// The first two bytes are the transaction id and they are not used in MDNS
	buffer[pos++] = 0;
	buffer[pos++] = 0;

	// 2 bytes for Flags
	buffer[pos++] = 0; // 0b00000000 for Query, 0b10000000 for Answer.
	buffer[pos++] = 0;

	// 2 bytes for number of questions
	buffer[pos++] = 0;
	buffer[pos++] = 1; // one

	// 2 bytes for number of Answer RRs
	buffer[pos++] = 0;
	buffer[pos++] = 0;

	// 2 bytes for Authority PRs
	buffer[pos++] = 0;
	buffer[pos++] = 0;

	// 2 bytes for Additional PRs
	buffer[pos++] = 0;
	buffer[pos++] = 0;

	uint16_t word_start{0};
	uint16_t word_end{0};

	const char* name = query.name;
	while(true) {
		if(name[word_end] == '.' || name[word_end] == '\0') {
			const uint8_t word_length = word_end - word_start;

			buffer[pos++] = word_length;
			memcpy(&buffer[pos], &name[word_start], word_length);
			pos += word_length;
			if(name[word_end] == '\0') {
				break;
			}
			word_end++; // Skip the '.' character.
			word_start = word_end;
		}
		word_end++;
	}

	buffer[pos++] = '\0'; // End of name.

	// 2 bytes for type
	buffer[pos++] = (query.type & 0xFF00) >> 8;
	buffer[pos++] = query.type & 0xFF;

	// 2 bytes for class
	unsigned int qclass = 0;
	if(query.isUnicastResponse) {
		qclass = 0b1000000000000000;
	}
	qclass += query.klass;
	buffer[pos++] = (qclass & 0xFF00) >> 8;
	buffer[pos++] = qclass & 0xFF;

	initialise();
	listen(0);
	return sendTo(IpAddress(MDNS_IP), MDNS_TARGET_PORT, reinterpret_cast<const char*>(buffer), pos);
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
	// debug_d("Finder::onReceive(%u)", buf->len);

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

	// Number of incoming queries.
	uint16_t questionsCount = (data[4] << 8) + data[5];
	if(questionsCount > 0) {
		// we are interested only in responses.
		return;
	}

	// Number of incoming answers.
	uint16_t answersCount = (data[6] << 8) + data[7];

	// Number of incoming Name Server resource records.
	uint16_t nsCount = (data[8] << 8) + data[9];

	// Number of incoming Additional resource records.
	uint16_t additionalCount = (data[10] << 8) + data[11];

	uint16_t pos = 12; // starting from the 12 byte we should have our answers
	for(uint16_t i = 0; i < (answersCount + nsCount + additionalCount); i++) {
		Answer answer;

		pos = nameFromDnsPointer(answer.name, 0, MAX_MDNS_NAME_LEN, data, pos);

		answer.type = data[pos++] << 8;
		answer.type += data[pos++];

		uint8_t rrclass_0 = data[pos++];
		uint8_t rrclass_1 = data[pos++];
		answer.isCachedFlush = (0b10000000 & rrclass_0);
		answer.klass = ((rrclass_0 & 0b01111111) << 8) + rrclass_1;

		answer.ttl = (data[pos++] << 24);
		answer.ttl += (data[pos++] << 16);
		answer.ttl += (data[pos++] << 8);
		answer.ttl += data[pos++];

		if(pos > size_t(size)) {
			// We've over-run the returned data.
			// Something has gone wrong receiving or parsing the data.
			answer.isValid = false;
			return;
		}

		uint16_t rdlength = data[pos++] << 8;
		rdlength += data[pos++];

		switch(answer.type) {
		case MDNS_TYPE_A: // Returns a 32-bit IPv4 address
			if(MAX_MDNS_NAME_LEN >= 16) {
				sprintf(answer.data, "%u.%u.%u.%u", data[pos], data[pos + 1], data[pos + 2], data[pos + 3]);
			} else {
				sprintf(answer.data, "ipv4");
			}
			pos += 4;
			break;

		case MDNS_TYPE_PTR: // Pointer to a canonical name.
			pos = nameFromDnsPointer(answer.data, 0, MAX_MDNS_NAME_LEN, data, pos);
			break;

		case MDNS_TYPE_HINFO: // HINFO. host information
			pos = parseText(answer.data, MAX_MDNS_NAME_LEN, rdlength, data, pos);
			break;

		case MDNS_TYPE_TXT: // Originally for arbitrary human-readable text in a DNS record.
			// We only return the first MAX_MDNS_NAME_LEN bytes of thir record type.
			pos = parseText(answer.data, MAX_MDNS_NAME_LEN, rdlength, data, pos);
			break;

		case MDNS_TYPE_AAAA: { // Returns a 128-bit IPv6 address.
			uint16_t data_pos{0};
			for(uint16_t i = 0; i < rdlength; i++) {
				if(data_pos < MAX_MDNS_NAME_LEN - 3) {
					sprintf(answer.data + data_pos, "%02X:", data[pos++]);
				} else {
					pos++;
				}
				data_pos += 3;
			}
			answer.data[--data_pos] = '\0'; // Remove trailing ':'
			break;
		}

		case MDNS_TYPE_SRV: { // Server Selection.
			unsigned int priority = (data[pos++] << 8);
			priority += data[pos++];
			unsigned int weight = (data[pos++] << 8);
			weight += data[pos++];
			unsigned int port = (data[pos++] << 8);
			port += data[pos++];
			sprintf(answer.data, "p=%u;w=%u;port=%u;host=", priority, weight, port);

			pos = nameFromDnsPointer(answer.data, strlen(answer.data), MAX_MDNS_NAME_LEN - strlen(answer.data) - 1,
									 data, pos);
			break;
		}

		default:
			uint16_t data_pos{0};
			for(uint16_t i = 0; i < rdlength; i++) {
				if(data_pos < MAX_MDNS_NAME_LEN - 3) {
					sprintf(answer.data + data_pos, "%02X ", data[pos++]);
				} else {
					pos++;
				}
				data_pos += 3;
			}
		}

		answer.isValid = true;

		if(answerCallback) {
			answerCallback(answer);
		}
	}
}

bool Finder::writeToBuffer(uint8_t value, char* name, uint16_t& namePos, uint16_t nameLength)
{
	if(namePos + 1 < nameLength) {
		name[namePos++] = value;
		name[namePos] = '\0';
		return true;
	}

	namePos++;
	return false;
}

uint16_t Finder::parseText(char* buffer, uint16_t bufferLength, uint16_t dataLength, const uint8_t* packet,
						   uint16_t packetPos)
{
	uint16_t bufferPos{0};
	for(uint16_t i = 0; i < dataLength; i++) {
		writeToBuffer(packet[packetPos++], buffer, bufferPos, bufferLength);
	}
	buffer[bufferPos] = '\0';
	return packetPos;
}

uint16_t Finder::nameFromDnsPointer(char* name, uint16_t namePos, uint16_t nameLength, const uint8_t* packet,
									uint16_t packetPos, bool recurse)
{
	if(recurse) {
		// Since we are adding more to an already populated buffer,
		// replace the trailing EOL with the FQDN seperator.
		namePos--;
		writeToBuffer('.', name, namePos, nameLength);
	}

	if(packet[packetPos] < 0xC0) {
		// Since the first 2 bits are not set,
		// this is the start of a name section.
		// http://www.tcpipguide.com/free/t_DNSNameNotationandMessageCompressionTechnique.htm

		const uint8_t word_len = packet[packetPos++];
		for(uint8_t l = 0; l < word_len; l++) {
			writeToBuffer(packet[packetPos++], name, namePos, nameLength);
		}

		writeToBuffer('\0', name, namePos, nameLength);

		if(packet[packetPos] > 0) {
			// Next word.
			packetPos = nameFromDnsPointer(name, namePos, nameLength, packet, packetPos, true);
		} else {
			// End of string.
			packetPos++;
		}
	} else {
		// Message Compression used. Next 2 bytes are a pointer to the actual name section.
		uint16_t pointer = (packet[packetPos++] - 0xC0) << 8;
		pointer += packet[packetPos++];
		nameFromDnsPointer(name, namePos, nameLength, packet, pointer, false);
	}
	return packetPos;
}

} // namespace mDNS
