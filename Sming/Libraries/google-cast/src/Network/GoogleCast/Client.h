#pragma once

#include <SmingCore.h>
#include <Data/Buffer/CircularBuffer.h>
#include <Data/CStringArray.h>
#include <ArduinoJson.h>
#include <PbUtils.h>
#include "cast_channel.pb.h"

typedef Delegate<bool(extensions_api_cast_channel_CastMessage)> CastMessageDelegate;

namespace GoogleCast
{
/*
 *
 * See: https://developers.google.com/cast/docs/reference/messages , https://github.com/thibauts/node-castv2
 */

#define CAST_CHANNEL_TYPE_MAP(XX)                                                                                      \
	/* Type, Namespace */                                                                                              \
	XX(CONNECTION, "urn:x-cast:com.google.cast.tp.connection")                                                         \
	XX(HEARTBEAT, "urn:x-cast:com.google.cast.tp.heartbeat")                                                           \
	XX(RECEIVER, "urn:x-cast:com.google.cast.receiver")                                                                \
	XX(MEDIA, "urn:x-cast:com.google.cast.media")

enum ChannelType {
#define XX(type, ns) CAST_CHANNEL_##type,
	CAST_CHANNEL_TYPE_MAP(XX)
#undef XX
};

// channel type strings
#define XX(type, ns) ns "\0"
DEFINE_FSTR_LOCAL(fstrChannelType, CAST_CHANNEL_TYPE_MAP(XX))
#undef XX

/** @brief Get textual representation for a MIME type
 *  @param m the MIME type
 *  @retval String
 */
String channelToString(enum ChannelType type)
{
	return CStringArray(fstrChannelType)[type];
}

constexpr char STANDARD_SENDER[] = "sender-0";
constexpr char STANDARD_RECEIVER[] = "receiver-0";

class Client : protected TcpClient
{
public:
	Client(size_t storageSize = 1024, bool autoDestruct = false)
		: TcpClient(autoDestruct), inputBuffer(new CircularBuffer(storageSize))
	{
		TcpClient::setReceiveDelegate(TcpClientDataDelegate(&Client::onTcpReceive, this));
	}

	bool connect(const IpAddress addr, int port = 8009)
	{
		TcpClient::setSslInitHandler([](Ssl::Session& session) {
			/* Ignore SSL certificate checking */
			session.options.verifyLater = true;
		});

		return TcpClient::connect(addr, port, true);
	}

	/**
	 * @brief  Tries to auto-discover Google Cast enabled TV and connect to it.
	 */
	bool connect()
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

	void setResponseHandler(CastMessageDelegate handler)
	{
		onMessage = handler;
	}

	/**
	 * Sets the interval in which to ping the remote server if there was no activity
	 * @param seconds
	 */
	void setPingRepeatTime(unsigned seconds)
	{
		pingRepeatTime = seconds;
	}

	/* High Level Commands */
	bool launch(const String& appId)
	{
		StaticJsonDocument<200> doc;
		doc["type"] = "LAUNCH";
		doc["appId"] = appId;
		doc["requestId"] = 1;

		return publish(doc);
	}

	bool play(const Url& url, const MimeType& mime)
	{
		return play(url, toString(mime));
	}

	bool play(const Url& url, const String& mime)
	{
		StaticJsonDocument<1024> media;
		media["contentId"] = url.toString();
		media["contentType"] = mime;
		media["streamType"] = "BUFFERED";

		StaticJsonDocument<1024> doc;
		doc["type"] = "LOAD";
		doc["autoplay"] = true;
		doc["currentTime"] = 0;
		doc.createNestedArray("activeTrackIds");
		doc["repeatMode"] = "REPEAT_OFF";
		doc["media"] = media;
		doc["requestId"] = 1;

		String command = generateMessage(Json::serialize(doc).c_str(), CAST_CHANNEL_MEDIA);
		return send(command.c_str(), command.length());
	}

	bool stop(const String& sessionId)
	{
		StaticJsonDocument<200> doc;
		doc["type"] = "STOP";
		doc["sessionId"] = sessionId.c_str();

		return publish(doc);
	}

	bool getStatus()
	{
		StaticJsonDocument<200> doc;
		doc["type"] = "STATUS";

		return publish(doc);
	}

	bool isAppAvailable(Vector<String> appIds);

	/**
	 * @brief: Sets volume.
	 * @param level is a float between 0 and 1
	 *
	 * @return bool
	 */
	bool setVolumeLevel(float level)
	{
		StaticJsonDocument<200> doc;
		doc["type"] = F("SET_VOLUME");
		doc["level"] = level;

		return publish(doc);
	}

	bool setVolumeMuted(bool muted)
	{
		StaticJsonDocument<200> doc;
		doc["type"] = F("SET_VOLUME");
		doc["volume"] = muted;

		return publish(doc);
	}

	/* Low Level Commands */

	/**
	 * @brief Publishes message in the receiver channel
	 */
	bool publish(const String& data)
	{
		String command = generateMessage(data.c_str(), CAST_CHANNEL_RECEIVER);
		return send(command.c_str(), command.length());
	}

	/**
	 * @brief Publishes message in the receiver channel
	 */
	bool publish(JsonDocument& json)
	{
		return publish(Json::serialize(json));
	}

	bool ping()
	{
		String command = generateMessage("{\"type\":\"PING\"}", CAST_CHANNEL_HEARTBEAT);
		lastPing = millis();
		return send(command.c_str(), command.length());
	}

	void close() override
	{
		String command = generateMessage("{\"type\":\"CLOSE\"}", CAST_CHANNEL_CONNECTION);
		send(command.c_str(), command.length());

		TcpClient::close();
	}

	~Client()
	{
	}

protected:
	err_t onConnected(err_t err) override
	{
		TcpClient::onConnected(err);
		if(getConnectionState() != eTCS_Connected) {
			return err;
		}

		//
		String command = generateMessage("{ \"type\": \"CONNECT\" }", CAST_CHANNEL_CONNECTION);
		send(command.c_str(), command.length());
		ping();

		return ERR_OK;
	}

	err_t onPoll() override
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

private:
	String generateMessage(const char* data, ChannelType type, const String& sourceId = STANDARD_SENDER,
						   const String& destinationId = STANDARD_RECEIVER)
	{
		return generateMessage((const uint8_t*)data, strlen(data), type, sourceId, destinationId);
	}

	String generateMessage(const uint8_t* data, size_t length, ChannelType type,
						   const String& sourceId = STANDARD_SENDER, const String& destinationId = STANDARD_RECEIVER)
	{
		extensions_api_cast_channel_CastMessage message = extensions_api_cast_channel_CastMessage_init_default;

		String ns = channelToString(type);
		message.protocol_version = extensions_api_cast_channel_CastMessage_ProtocolVersion_CASTV2_1_0;
		message.source_id.funcs.encode = &pbEncodeData;
		message.source_id.arg = new PbData(sourceId);
		message.destination_id.funcs.encode = &pbEncodeData;
		message.destination_id.arg = new PbData(destinationId);
		message.nameSpace.funcs.encode = &pbEncodeData;
		message.nameSpace.arg = new PbData(ns);
		message.payload_type = extensions_api_cast_channel_CastMessage_PayloadType_STRING;
		message.payload_utf8.funcs.encode = &pbEncodeData;
		message.payload_utf8.arg = new PbData((uint8_t*)data, length);

		uint8_t* buf = nullptr;
		uint32_t bufferSize = 0;
		uint8_t packetSize[4];
		boolean status;
		String packet;

		pb_ostream_t stream;

		do {
			if(buf) {
				delete[] buf;
			}
			bufferSize += 1024;
			buf = new uint8_t[bufferSize];

			stream = pb_ostream_from_buffer(buf, bufferSize);
			status = pb_encode(&stream, extensions_api_cast_channel_CastMessage_fields, &message);
		} while(status == false && bufferSize < 10240);

		delete(PbData*)message.source_id.arg;
		delete(PbData*)message.destination_id.arg;
		delete(PbData*)message.nameSpace.arg;
		delete(PbData*)message.payload_utf8.arg;

		if(status == false) {
			char error[128];
			debug_e("Failed to encode. (source_id=%s, destination_id=%s, namespace=%s, data=%s)", sourceId.c_str(),
					destinationId.c_str(), ns.c_str(), data);
			return packet;
		}

		bufferSize = stream.bytes_written;
		for(int i = 0; i < 4; i++) {
			packetSize[3 - i] = (bufferSize >> 8 * i) & 0x000000FF;
		}

		packet.concat((const char*)packetSize, 4);
		packet.concat((const char*)buf, bufferSize);

		delete[] buf;

		return packet;
	}

	pb_istream_t newInputStream()
	{
		pb_istream_t stream;
		stream.callback = [](pb_istream_t* stream, pb_byte_t* buf, size_t count) -> bool {
			CircularBuffer* source = (CircularBuffer*)stream->state;
			size_t read = source->readMemoryBlock((char*)buf, count);
			source->seek(read);

			return true;
		};
		stream.state = (void*)inputBuffer;
		stream.bytes_left = inputBuffer->available();
		stream.errmsg = nullptr;

		return stream;
	}

	bool onTcpReceive(TcpClient& client, char* data, int length)
	{
		size_t written = inputBuffer->write((const uint8_t*)data, length);
		if(written != length) {
			debug_e("Not enough space to store the message...");
			return false;
		}

		// parse the incoming message....
		if(!messageLength) {
			if(inputBuffer->available() < 4) {
				// we don't have all needed starting bytes yet...
				return true;
			}

			// get the message length from the first 4 bytes
			uint8_t header[4];
			inputBuffer->readBytes((char*)header, 4);
			for(int i = 0; i < 4; i++) {
				messageLength |= header[i] << 8 * (3 - i);
			}
		}

		if(inputBuffer->available() < messageLength) {
			// still waiting for the rest of the needed bytes...
			return true;
		}

		extensions_api_cast_channel_CastMessage message = extensions_api_cast_channel_CastMessage_init_default;
		message.source_id.funcs.decode = &pbDecodeData;
		message.destination_id.funcs.decode = &pbDecodeData;
		message.nameSpace.funcs.decode = &pbDecodeData;
		message.payload_utf8.funcs.decode = &pbDecodeData;

		pb_istream_t input = newInputStream();
		size_t leftBytes = input.bytes_left;
		bool success = pb_decode(&input, extensions_api_cast_channel_CastMessage_fields, &message);
		messageLength = 0;
		if(!success) {
			debug_e("Decoding failed: %s\n", PB_GET_ERROR(&input));
			return false;
		}

		// JSON decode the message payload. Pass the message to an event handler....
		if(onMessage) {
			MemoryDataStream* tmp = (MemoryDataStream*)message.source_id.arg;
			if(tmp != nullptr) {
				char value[tmp->available()];
				tmp->readBytes(value, tmp->available());
				delete tmp;
				message.source_id.arg = (void*)value;
			}

			tmp = (MemoryDataStream*)message.destination_id.arg;
			if(tmp != nullptr) {
				char value[tmp->available()];
				tmp->readBytes(value, tmp->available());
				delete tmp;
				message.destination_id.arg = (void*)value;
			}

			tmp = (MemoryDataStream*)message.destination_id.arg;
			if(tmp != nullptr) {
				char value[tmp->available()];
				tmp->readBytes(value, tmp->available());
				delete tmp;

				if((message.payload_type == extensions_api_cast_channel_CastMessage_PayloadType_STRING)) {
					DynamicJsonDocument doc(1024);
					Json::deserialize(doc, value, strlen(value));

					message.payload_utf8.arg = (void*)&doc;
				} else {
					message.payload_utf8.arg = value;
				}
			}

			return onMessage(message);
		}

		return true;
	}

private:
	unsigned lastPing = 0;
	unsigned pingRepeatTime = 4;

	size_t messageLength = 0;
	CircularBuffer* inputBuffer = nullptr;
	CastMessageDelegate onMessage = nullptr;
};

} // namespace GoogleCast
