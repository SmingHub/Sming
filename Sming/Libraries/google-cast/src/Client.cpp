#include "Network/GoogleCast/Client.h"
#include <Data/CStringArray.h>
#include <PbUtils.h>

namespace
{
// channel type strings
#define XX(type, ns) ns "\0"
DEFINE_FSTR(fstrChannelType, CAST_CHANNEL_TYPE_MAP(XX))
#undef XX

DEFINE_FSTR(STANDARD_SENDER, "sender-0")
DEFINE_FSTR(STANDARD_RECEIVER, "receiver-0")

pb_istream_t newInputStream(IDataSourceStream* inputBuffer)
{
	pb_istream_t stream;
	stream.callback = [](pb_istream_t* stream, pb_byte_t* buf, size_t count) -> bool {
		auto source = static_cast<IDataSourceStream*>(stream->state);
		size_t read = source->readBytes(reinterpret_cast<char*>(buf), count);
		return true;
	};
	stream.state = inputBuffer;
	stream.bytes_left = inputBuffer->available();
	stream.errmsg = nullptr;

	return stream;
}

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

bool Client::play(const Url& url, const String& mime)
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

	String command = generateMessage(Json::serialize(doc), ChannelType::MEDIA);
	return send(command.c_str(), command.length());
}

bool Client::stop(const String& sessionId)
{
	StaticJsonDocument<200> doc;
	doc[F("type")] = F("STOP");
	doc[F("sessionId")] = sessionId.c_str();

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

bool Client::publish(const String& data)
{
	String command = generateMessage(data, ChannelType::RECEIVER);
	return sendString(command);
}

bool Client::publish(JsonDocument& json)
{
	return Client::publish(Json::serialize(json));
}

bool Client::ping()
{
	String command = generateMessage(F("{\"type\":\"PING\"}"), ChannelType::HEARTBEAT);
	lastPing = millis();
	return send(command.c_str(), command.length());
}

void Client::close()
{
	String command = generateMessage(F("{\"type\":\"CLOSE\"}"), ChannelType::CONNECTION);
	send(command.c_str(), command.length());

	TcpClient::close();
}

err_t Client::onConnected(err_t err)
{
	TcpClient::onConnected(err);
	if(getConnectionState() != eTCS_Connected) {
		return err;
	}

	//
	String command = generateMessage(F("{ \"type\": \"CONNECT\" }"), ChannelType::CONNECTION);
	send(command.c_str(), command.length());
	ping();

	return ERR_OK;
}

err_t Client::onPoll()
{
	err_t err = TcpClient::onPoll();
	if(err != ERR_OK) {
		return err;
	}

	// ping the server if it is high time ...
	if(!(lastPing && (millis() - lastPing >= pingRepeatTime * 1000))) {
		ping();
	}

	return ERR_OK;
}

String Client::generateMessage(const uint8_t* data, size_t length, ChannelType type, const String& sourceId,
							   const String& destinationId)
{
	extensions_api_cast_channel_CastMessage message = extensions_api_cast_channel_CastMessage_init_default;

	String ns = toString(type);
	message.protocol_version = extensions_api_cast_channel_CastMessage_ProtocolVersion_CASTV2_1_0;
	message.source_id.funcs.encode = &pbEncodeData;
	message.source_id.arg = new PbData(sourceId ?: STANDARD_SENDER);
	message.destination_id.funcs.encode = &pbEncodeData;
	message.destination_id.arg = new PbData(destinationId ?: STANDARD_RECEIVER);
	message.nameSpace.funcs.encode = &pbEncodeData;
	message.nameSpace.arg = new PbData(ns);
	message.payload_type = extensions_api_cast_channel_CastMessage_PayloadType_STRING;
	message.payload_utf8.funcs.encode = &pbEncodeData;
	message.payload_utf8.arg = new PbData(const_cast<uint8_t*>(data), length);

	pb_ostream_t stream;
	uint8_t* buf{nullptr};
	uint32_t bufferSize{0};
	bool status;
	do {
		if(buf != nullptr) {
			delete[] buf;
		}
		bufferSize += 1024;
		buf = new uint8_t[bufferSize];

		stream = pb_ostream_from_buffer(buf, bufferSize);
		status = pb_encode(&stream, extensions_api_cast_channel_CastMessage_fields, &message);
	} while(!status && bufferSize < 10240);

	delete(PbData*)message.source_id.arg;
	delete(PbData*)message.destination_id.arg;
	delete(PbData*)message.nameSpace.arg;
	delete(PbData*)message.payload_utf8.arg;

	if(!status) {
		char error[128];
		debug_e("Failed to encode. (source_id=%s, destination_id=%s, namespace=%s, data=%s)", sourceId.c_str(),
				destinationId.c_str(), ns.c_str(), data);
		return nullptr;
	}

	String packet;
	packet.reserve(sizeof(uint32_t) + bufferSize);
	uint32_t packetSize = htonl(stream.bytes_written);
	packet.concat(reinterpret_cast<const char*>(&packetSize), sizeof(packetSize));
	packet.concat((const char*)buf, bufferSize);

	delete[] buf;

	return packet;
}

bool Client::onTcpReceive(TcpClient& client, char* data, int length)
{
	size_t written = inputBuffer->write((const uint8_t*)data, length);
	if(written != size_t(length)) {
		debug_e("Not enough space to store the message...");
		return false;
	}

	// parse the incoming message....
	if(messageLength == 0) {
		if(inputBuffer->available() < 4) {
			// we don't have all needed starting bytes yet...
			return true;
		}

		// get the message length from the first 4 bytes
		uint32_t tmp;
		inputBuffer->readBytes(reinterpret_cast<char*>(&tmp), sizeof(tmp));
		messageLength = ntohl(tmp);
	}

	if(inputBuffer->available() < int(messageLength)) {
		// still waiting for the rest of the needed bytes...
		return true;
	}

	extensions_api_cast_channel_CastMessage message = extensions_api_cast_channel_CastMessage_init_default;
	message.source_id.funcs.decode = &pbDecodeData;
	message.destination_id.funcs.decode = &pbDecodeData;
	message.nameSpace.funcs.decode = &pbDecodeData;
	message.payload_utf8.funcs.decode = &pbDecodeData;

	pb_istream_t input = newInputStream(inputBuffer);
	size_t leftBytes = input.bytes_left;
	bool success = pb_decode(&input, extensions_api_cast_channel_CastMessage_fields, &message);
	messageLength = 0;
	if(!success) {
		debug_e("Decoding failed: %s\n", PB_GET_ERROR(&input));
		return false;
	}

	// JSON decode the message payload. Pass the message to an event handler....
	if(onMessage) {
		auto tmp = static_cast<MemoryDataStream*>(message.source_id.arg);
		if(tmp != nullptr) {
			char value[tmp->available()];
			tmp->readBytes(value, tmp->available());
			delete tmp;
			message.source_id.arg = value;
		}

		tmp = static_cast<MemoryDataStream*>(message.destination_id.arg);
		if(tmp != nullptr) {
			char value[tmp->available()];
			tmp->readBytes(value, tmp->available());
			delete tmp;
			message.destination_id.arg = value;
		}

		tmp = static_cast<MemoryDataStream*>(message.destination_id.arg);
		if(tmp != nullptr) {
			char value[tmp->available()];
			tmp->readBytes(value, tmp->available());
			delete tmp;

			if((message.payload_type == extensions_api_cast_channel_CastMessage_PayloadType_STRING)) {
				DynamicJsonDocument doc(1024);
				Json::deserialize(doc, value, strlen(value));

				message.payload_utf8.arg = &doc;
			} else {
				message.payload_utf8.arg = value;
			}
		}

		return onMessage(message);
	}

	return true;
}

} // namespace GoogleCast
