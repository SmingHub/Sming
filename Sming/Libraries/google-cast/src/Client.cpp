/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Client.cpp
 *
 ****/

#include "Network/GoogleCast/Client.h"
#include <Data/Stream/LimitedMemoryStream.h>

namespace GoogleCast
{
bool Client::connect(const IpAddress addr, int port)
{
	TcpClient::setSslInitHandler([](Ssl::Session& session) {
		/* Ignore SSL certificate checking */
		session.options.verifyLater = true;
	});

	return TcpClient::connect(addr, port, true);
}

bool Client::connect()
{
	/**
	 * Discoveries...
	 * sudo netstat -anp|grep 5353
	 *
	 * dig @224.0.0.251 -p 5353 -t PTR _googlecast._tcp.local.
	 * avahi-browse -tr _googlecast._tcp
	 */

	// TODO: Use MDNS and query for service googlecast over tcp -> find the ipaddress and port.
	return false;
}

bool Client::ping()
{
	pingTimer.start();
	return heartbeat.ping();
}

void Client::close()
{
	connection.close();
	TcpClient::close();
}

err_t Client::onConnected(err_t err)
{
	TcpClient::onConnected(err);
	if(getConnectionState() != eTCS_Connected) {
		return err;
	}

	connection.connect();
	ping();

	return ERR_OK;
}

err_t Client::onPoll()
{
	err_t err = TcpClient::onPoll();
	if(err != ERR_OK) {
		return err;
	}

	if(pingTimer.expired()) {
		ping();
	}

	return ERR_OK;
}

bool Client::sendMessage(const void* data, size_t length, const String& nameSpace, const String& sourceId,
						 const String& destinationId)
{
#if DEBUG_VERBOSE_LEVEL >= INFO
	m_printf("%u send: ", system_get_time());
	m_nputs((char*)data, length);
	m_puts(" to ");
	m_puts(nameSpace.c_str());
	m_puts("\r\n");
#endif

	extensions_api_cast_channel_CastMessage message = extensions_api_cast_channel_CastMessage_init_default;

	message.protocol_version = extensions_api_cast_channel_CastMessage_ProtocolVersion_CASTV2_1_0;
	Protobuf::OutputCallback cbSourceId(message.source_id, sourceId ?: STANDARD_SENDER);
	Protobuf::OutputCallback cbDestinationId(message.destination_id, destinationId ?: STANDARD_RECEIVER);
	Protobuf::OutputCallback cbNameSpace(message.nameSpace, nameSpace);
	message.payload_type = extensions_api_cast_channel_CastMessage_PayloadType_STRING;
	Protobuf::OutputCallback cbPayload(message.payload_utf8, data, length);

	// Calculate and output packet size field first
	uint32_t packetSize = Protobuf::getEncodeSize(extensions_api_cast_channel_CastMessage_fields, &message);
	packetSize = htonl(packetSize);
	int err = send(reinterpret_cast<const char*>(&packetSize), sizeof(packetSize));

	// Now stream the encoded data
	Protobuf::TcpClientOutputStream stream(*this);
	auto sent = stream.encode(extensions_api_cast_channel_CastMessage_fields, &message);
	if(sent > 0) {
		return true;
	}

	debug_e("Failed to encode. (source_id='%s', destination_id='%s', namespace='%s', data='%s')",
			String(cbSourceId).c_str(), String(cbDestinationId).c_str(), String(cbNameSpace).c_str(), data);
	return false;
}

bool Client::onTcpReceive(TcpClient& client, char* data, int length)
{
	uint32_t messageLength{0};

	if(inputBuffer == nullptr) {
		constexpr auto headerSize{sizeof messageLength};
		if(length < int(headerSize)) {
			// Not enough to read header. Should never happen.
			debug_e("Unexpected: Got %d bytes but need at least %u", length, headerSize);
			return false;
		}

		// get the message length
		memcpy(&messageLength, data, headerSize);
		messageLength = ntohl(messageLength);
		data += headerSize;
		length -= headerSize;

		// Create stream buffer if we need more data
		if(length < int(messageLength)) {
			inputBuffer = new LimitedMemoryStream(messageLength);

			if(inputBuffer == nullptr) {
				debug_e("Not enough memory");
				return false;
			}
		}
	} else {
		messageLength = inputBuffer->getCapacity();
	}

	if(inputBuffer == nullptr) {
		inputBuffer = new LimitedMemoryStream(data, messageLength, messageLength, false);
	} else {
		size_t written = inputBuffer->write((const uint8_t*)data, length);
		if(written != size_t(length)) {
			debug_e("Unexpected: Data overran input buffer");
			return false;
		}

		if(inputBuffer->available() < int(messageLength)) {
			// More data required
			return true;
		}
	}

	Channel::Message message;
	Protobuf::InputStream input(*inputBuffer);
	bool success = message.decode(input);
	delete inputBuffer;
	inputBuffer = nullptr;
	if(!success) {
		debug_e("Decoding failed: %s", input.getErrorString().c_str());
		return false;
	}

	if(!dispatch(message)) {
		// Message not handled (not an error)
		if(callback) {
			callback(message);
		}
	}

	return true;
}

bool Client::dispatch(Channel::Message& message)
{
	for(auto& c : channels) {
		if(message.nameSpace == c.getNameSpace()) {
			return c.handleMessage(message);
		}
	}

	// Not handled
	return false;
}

} // namespace GoogleCast
