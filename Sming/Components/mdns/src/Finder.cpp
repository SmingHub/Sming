#include "include/Network/Mdns/Finder.h"
#include "include/Network/Mdns/Response.h"
#include "Packet.h"
#include <IFS/FileSystem.h>

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
	Query query{};
	query.name = hostname;
	query.type = type;
	query.klass = 1; // "INternet"
	query.isUnicastResponse = false;
	query.isValid = true;

	return search(query);
}

bool Finder::search(const Query& query)
{
	if(query.name.length() > MAX_MDNS_NAME_LEN - 1) {
		return false;
	}

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

	size_t pos{0};
	auto& name = query.name;
	auto namelen = name.length();
	do {
		int sep = name.indexOf('.', pos);
		auto wordLength = (sep >= 0) ? (sep - pos) : (namelen - pos);
		pkt.write8(wordLength);
		pkt.write(name.c_str() + pos, wordLength);
		pos = sep + 1;
	} while(pos > 0);

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
	// auto& hostfs = IFS::Host::getFileSystem();
	// String filename = remoteIP.toString();
	// filename += ".mdns";
	// auto f = hostfs.open(filename, IFS::OpenFlag::Create | IFS::OpenFlag::Truncate | IFS::OpenFlag::Write);
	// hostfs.write(f, buf->payload, buf->len);
	// hostfs.close(f);
	// debug_i("Saved '%s'", filename.c_str());
	// return;

	if(!answerCallback) {
		return;
	}

	Response response(buf->payload, buf->len);
	response.parse();
	answerCallback(response);
}

} // namespace mDNS
