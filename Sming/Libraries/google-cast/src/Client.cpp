#include "Network/GoogleCast/Client.h"
#include <Data/CStringArray.h>
#include <Protobuf.h>

namespace
{
// channel type strings
#define XX(type, ns) ns "\0"
DEFINE_FSTR(fstrChannelType, CAST_CHANNEL_TYPE_MAP(XX))
#undef XX

DEFINE_FSTR(STANDARD_SENDER, "sender-0")
DEFINE_FSTR(STANDARD_RECEIVER, "receiver-0")

} // namespace

String toString(enum GoogleCast::ChannelType type)
{
	return CStringArray(fstrChannelType)[unsigned(type)];
}

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

bool Client::launch(const String& appId)
{
	StaticJsonDocument<200> doc;
	doc[F("type")] = F("LAUNCH");
	doc[F("appId")] = appId;
	doc[F("requestId")] = 1;

	return publish(doc);
}

bool Client::load(const Url& url, const String& mime)
{
	StaticJsonDocument<1024> doc;
	doc[F("type")] = F("LOAD");
	doc[F("autoplay")] = true;
	doc[F("currentTime")] = 0;
	doc.createNestedArray(F("activeTrackIds"));
	doc[F("repeatMode")] = F("REPEAT_OFF");
	auto media = doc.createNestedObject(F("media"));
	media[F("contentId")] = url.toString();
	media[F("contentType")] = mime;
	media[F("streamType")] = F("BUFFERED");
	doc[F("requestId")] = 1;

	return sendMessage(Json::serialize(doc), ChannelType::MEDIA);
}

bool Client::pause(const String& sessionId)
{
	StaticJsonDocument<200> doc;
	doc[F("type")] = F("PAUSE");
	doc[F("mediaSessionId")] = sessionId;
	doc[F("requestId")] = requestId++;

	return sendMessage(doc, ChannelType::MEDIA);
}

bool Client::play(const String& sessionId)
{
	StaticJsonDocument<200> doc;
	doc[F("type")] = F("PLAY");
	doc[F("mediaSessionId")] = sessionId;
	doc[F("requestId")] = requestId++;

	return sendMessage(doc, ChannelType::MEDIA);
}

bool Client::stop(const String& sessionId)
{
	StaticJsonDocument<200> doc;
	doc[F("type")] = F("STOP");
	doc[F("mediaSessionId")] = sessionId;
	doc[F("requestId")] = requestId++;

	return publish(doc);
}

bool Client::getStatus()
{
	StaticJsonDocument<200> doc;
	doc[F("type")] = F("STATUS");

	return publish(doc);
}

bool isAppAvailable(Vector<String> appIds)
{
	// TODO
	return false;
}

bool Client::setVolumeLevel(float level)
{
	StaticJsonDocument<200> doc;
	doc[F("type")] = F("SET_VOLUME");
	doc[F("level")] = level;

	return publish(doc);
}

bool Client::setVolumeMuted(bool muted)
{
	StaticJsonDocument<200> doc;
	doc[F("type")] = F("SET_VOLUME");
	doc[F("volume")] = muted;

	return publish(doc);
}

bool Client::ping()
{
	pingTimer.start();
	return sendMessage(makeMessage(F("PING")), ChannelType::HEARTBEAT);
}

void Client::close()
{
	sendMessage(F("{\"type\":\"CLOSE\"}"), ChannelType::CONNECTION);

	TcpClient::close();
}

err_t Client::onConnected(err_t err)
{
	TcpClient::onConnected(err);
	if(getConnectionState() != eTCS_Connected) {
		return err;
	}

	//
	sendMessage(F("{ \"type\": \"CONNECT\" }"), ChannelType::CONNECTION);
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

bool Client::sendMessage(const uint8_t* data, size_t length, ChannelType type, const String& sourceId,
						 const String& destinationId)
{
	extensions_api_cast_channel_CastMessage message = extensions_api_cast_channel_CastMessage_init_default;

	message.protocol_version = extensions_api_cast_channel_CastMessage_ProtocolVersion_CASTV2_1_0;
	Protobuf::OutputCallback source_id(message.source_id, sourceId ?: STANDARD_SENDER);
	Protobuf::OutputCallback destination_id(message.destination_id, destinationId ?: STANDARD_RECEIVER);
	Protobuf::OutputCallback nameSpace(message.nameSpace, toString(type));
	message.payload_type = extensions_api_cast_channel_CastMessage_PayloadType_STRING;
	Protobuf::OutputCallback payload_utf8(message.payload_utf8, data, length);

	// Calculate and output packet size field first
	uint32_t packetSize = Protobuf::getEncodeSize(extensions_api_cast_channel_CastMessage_fields, &message);
	// debug_i("packetSize = %u", packetSize);
	packetSize = htonl(packetSize);
	int err = send(reinterpret_cast<const char*>(&packetSize), sizeof(packetSize));
	// debug_i("write(%u): %d", sizeof(packetSize), err);

	// Now stream the encoded data
	Protobuf::TcpClientOutputStream stream(*this);
	auto sent = stream.encode(extensions_api_cast_channel_CastMessage_fields, &message);
	// debug_i("stream.encode(): %u", sent);
	if(sent > 0) {
		return true;
	}

	debug_e("Failed to encode. (source_id='%s', destination_id='%s', namespace='%s', data='%s')",
			String(source_id).c_str(), String(destination_id).c_str(), String(nameSpace).c_str(), data);
	return false;
}

bool Client::onTcpReceive(TcpClient& client, char* data, int length)
{
	if(messageLength == 0) {
		if(length < int(sizeof(messageLength))) {
			// Not enough to read header. Should never happen.
			debug_e("Unexpected: Got %d bytes but need at least 4", length);
			return false;
		}

		// get the message length
		uint32_t tmp;
		memcpy(&tmp, data, sizeof(tmp));
		messageLength = ntohl(tmp);
		data += sizeof(tmp);
		length -= sizeof(tmp);

		assert(inputBuffer == nullptr);

		// Create stream buffer if we need more data
		if(length < int(messageLength)) {
			inputBuffer = new LimitedMemoryStream(messageLength);

			if(inputBuffer == nullptr) {
				debug_e("Not enough memory");
				return false;
			}
		}
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

	extensions_api_cast_channel_CastMessage message = extensions_api_cast_channel_CastMessage_init_default;
	Protobuf::InputCallback source_id(message.source_id);
	Protobuf::InputCallback destination_id(message.destination_id);
	Protobuf::InputCallback nameSpace(message.nameSpace);
	Protobuf::InputCallback payload_utf8(message.payload_utf8);
	Protobuf::InputStream input(*inputBuffer);
	bool success = input.decode(extensions_api_cast_channel_CastMessage_fields, &message);
	messageLength = 0;
	delete inputBuffer;
	inputBuffer = nullptr;
	if(!success) {
		debug_e("Decoding failed: %s", input.getErrorString().c_str());
		return false;
	}

	// JSON decode the message payload. Pass the message to an event handler....
	if(onMessage) {
		// tmp = static_cast<MemoryDataStream*>(message.source_id.arg);
		// if(tmp != nullptr) {
		// 	size_t len = tmp->available();
		// 	char value[len];
		// 	tmp->readBytes(value, len);
		// 	delete tmp;
		// 	message.source_id.arg = value;
		// }

		// tmp = static_cast<MemoryDataStream*>(message.destination_id.arg);
		// if(tmp != nullptr) {
		// 	size_t len = tmp->available();
		// 	char value[len];
		// 	tmp->readBytes(value, len);
		// 	delete tmp;
		// 	message.destination_id.arg = value;
		// }

		if((message.payload_type == extensions_api_cast_channel_CastMessage_PayloadType_STRING)) {
			auto value = payload_utf8.getData();
			auto len = payload_utf8.getLength();

			m_puts("Response: ");
			m_nputs(reinterpret_cast<const char*>(value), len);
			m_puts("\r\n");

			DynamicJsonDocument doc(1024);
			Json::deserialize(doc, value, len);
		}

		return onMessage(message);
	}

	return true;
}

} // namespace GoogleCast
