/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * MqttClient.cpp
 *
 ****/

#include "MqttClient.h"

#include "Data/Stream/MemoryDataStream.h"
#include "Data/Stream/StreamChain.h"

const mqtt_parser_callbacks_t MqttClient::callbacks PROGMEM = {
	.on_message_begin = staticOnMessageBegin,
	.on_data_begin = staticOnDataBegin,
	.on_data_payload = staticOnDataPayload,
	.on_data_end = staticOnDataEnd,
	.on_message_end = staticOnMessageEnd,
};

namespace
{
// Content length set to this value to indicate data refers to a stream, not a buffer
constexpr unsigned MQTT_PUBLISH_STREAM{0};

constexpr uint8_t MQTT_CONNECT_PROTOCOL{4}; // version 3.1.1

#define GET_CLIENT()                                                                                                   \
	auto client = static_cast<MqttClient*>(userData);                                                                  \
	if(client == nullptr) {                                                                                            \
		return -1;                                                                                                     \
	}

mqtt_message_t* createMessage(mqtt_type_t messageType)
{
	auto message = new mqtt_message_t;
	if(message != nullptr) {
		mqtt_message_init(message);
		message->common.type = messageType;
	}
	return message;
}

void deleteMessage(mqtt_message_t* message)
{
	mqtt_message_clear(message, 0);
	delete message;
}

void clearMessage(mqtt_message_t& message)
{
	mqtt_message_clear(&message, 0);
}

bool copyString(mqtt_buffer_t& destBuffer, const String& sourceString)
{
	destBuffer.length = sourceString.length();
	MQTT_FREE(destBuffer.data); // Avoid memory leaks
	destBuffer.data = (uint8_t*)MQTT_MALLOC(sourceString.length());
	if(destBuffer.data == nullptr) {
		debug_e("Not enough memory");
		return false;
	}
	memcpy(destBuffer.data, sourceString.c_str(), sourceString.length());
	return true;
}

} // namespace

MqttClient::MqttClient(bool withDefaultPayloadParser, bool autoDestruct)
	: TcpClient(autoDestruct), pingTimer(pingRepeatTime)
{
	mqtt_parser_init(&parser, const_cast<mqtt_parser_callbacks_t*>(&callbacks));
	mqtt_serialiser_init(&serialiser);
	mqtt_message_init(&incomingMessage);
	mqtt_message_init(&connectMessage);

	parser.data = this;
	connectMessage.common.type = MQTT_TYPE_CONNECT;
	connectMessage.connect.protocol_version = MQTT_CONNECT_PROTOCOL;

	if(withDefaultPayloadParser) {
		setPayloadParser(defaultPayloadParser);
	}

	TcpClient::setReceiveDelegate(TcpClientDataDelegate(&MqttClient::onTcpReceive, this));
}

MqttClient::~MqttClient()
{
	while(requestQueue.count() != 0) {
		deleteMessage(requestQueue.dequeue());
	}

	clearMessage(connectMessage);
	if(outgoingMessage != &connectMessage) {
		deleteMessage(outgoingMessage);
	}
	outgoingMessage = nullptr;
	clearMessage(incomingMessage);
}

bool MqttClient::onTcpReceive(TcpClient& client, char* data, int size)
{
	pingTimer.start();
	int rc = mqtt_parser_execute(&parser, &incomingMessage, (uint8_t*)data, (size_t)size);
	if(rc == MQTT_PARSER_RC_ERROR) {
		debug_e("MqttClient parse error: %s", mqtt_error_string(parser.error));
		return false;
	}

	return true;
}

int MqttClient::staticOnMessageBegin(void* userData, mqtt_message_t* message)
{
	// At that moment the message contains the type and its common length
	return 0;
}

int MqttClient::staticOnDataBegin(void* userData, mqtt_message_t* message)
{
	GET_CLIENT();

	if(client->payloadParser) {
		client->payloadState.offset = 0;
		return client->payloadParser(client->payloadState, message, nullptr, MQTT_PAYLOAD_PARSER_START);
	}

	return 0;
}

int MqttClient::staticOnDataPayload(void* userData, mqtt_message_t* message, const char* data, size_t length)
{
	GET_CLIENT();

	if(client->payloadParser) {
		return client->payloadParser(client->payloadState, message, data, length);
	}

	return 0;
}

int MqttClient::staticOnDataEnd(void* userData, mqtt_message_t* message)
{
	GET_CLIENT();

	if(client->payloadParser) {
		return client->payloadParser(client->payloadState, message, nullptr, MQTT_PAYLOAD_PARSER_END);
	}

	return 0;
}

int MqttClient::staticOnMessageEnd(void* userData, mqtt_message_t* message)
{
	GET_CLIENT();
	return client->onMessageEnd(message);
}

int MqttClient::onMessageEnd(mqtt_message_t* message)
{
	if(message->common.type == MQTT_TYPE_CONNACK) {
		if(message->connack.return_code) {
			// failure
			clearBits(flags, MQTT_CLIENT_CONNECTED);
			setTimeOut(1); // schedule the connection for closing

			return message->connack.return_code;
		}

		// success
		setTimeOut(USHRT_MAX);
		setBits(flags, MQTT_CLIENT_CONNECTED);
	}

	auto& handler = static_cast<const HandlerMap&>(eventHandlers)[message->common.type];
	if(handler) {
		return handler(*this, message);
	}

	return 0;
}

bool MqttClient::setWill(const String& topic, const String& message, uint8_t flags)
{
	if(bitsSet(this->flags, MQTT_CLIENT_CONNECTED)) {
		debug_e("Will must be set before connect");
		return false;
	}

	connectMessage.connect.flags.will_retain = (flags >> 0) & 0x01;
	connectMessage.connect.flags.will_qos = (flags >> 1) & 0x03;
	connectMessage.connect.flags.will = 1;

	return copyString(connectMessage.connect.will_topic, topic) &&
		   copyString(connectMessage.connect.will_message, message);
}

bool MqttClient::connect(const Url& url, const String& clientName)
{
	this->url = url;
	bool useSsl{url.Scheme == URI_SCHEME_MQTT_SECURE};
	if(!useSsl && url.Scheme != URI_SCHEME_MQTT) {
		debug_e("Only mqtt and mqtts protocols are allowed");
		return false;
	}

	if(getConnectionState() != eTCS_Ready) {
		close();
		debug_d("MQTT closed previous connection");
	}

	debug_d("MQTT start connection");

	bool res = copyString(connectMessage.connect.protocol_name, F("MQTT"));

	connectMessage.connect.keep_alive = keepAlive;

	res &= copyString(connectMessage.connect.client_id, clientName);

	if(url.User.length() > 0) {
		connectMessage.connect.flags.username_follows = 1;
		res &= copyString(connectMessage.connect.username, url.User);
		if(url.Password.length() > 0) {
			connectMessage.connect.flags.password_follows = 1;
			res &= copyString(connectMessage.connect.password, url.Password);
		}
	}

	if(!res) {
		debug_e("MQTT out of memory");
		return false;
	}

	// We'll pick up connectMessage before sending any other queued messages
	if(connectQueued) {
		debug_i("MQTT replacing connect message");
	}
	connectQueued = true;

	return TcpClient::connect(url.Host, url.getPort(), useSsl);
}

bool MqttClient::publish(const String& topic, const String& content, uint8_t flags)
{
	if(requestQueue.full()) {
		return false;
	}

	auto message = createMessage(MQTT_TYPE_PUBLISH);

	message->common.retain = static_cast<mqtt_retain_t>((flags >> 0) & 0x01);
	message->common.qos = static_cast<mqtt_qos_t>((flags >> 1) & 0x03);
	message->common.dup = static_cast<mqtt_dup_t>((flags >> 3) & 0x01);

	if(!copyString(message->publish.topic_name, topic) || !copyString(message->publish.content, content)) {
		delete message;
		return false;
	}

	bool success = requestQueue.enqueue(message);
	if(success) {
		// Try to force-send message to decrease latency.
		// Should work for small size messages but there is no guarantee.
		onReadyToSendData(TcpConnectionEvent::eTCE_Poll);
	}

	return success;
}

bool MqttClient::publish(const String& topic, IDataSourceStream* stream, uint8_t flags)
{
	if(!stream || stream->available() < 1) {
		debug_e("Sending empty stream or stream with unknown size is not supported");
		return false;
	}

	if(requestQueue.full()) {
		return false;
	}

	auto message = createMessage(MQTT_TYPE_PUBLISH);

	message->common.retain = static_cast<mqtt_retain_t>((flags >> 0) & 0x01);
	message->common.qos = static_cast<mqtt_qos_t>((flags >> 1) & 0x03);
	message->common.dup = static_cast<mqtt_dup_t>((flags >> 3) & 0x01);

	if(!copyString(message->publish.topic_name, topic)) {
		delete message;
		delete stream;
		return false;
	}

	message->publish.content.length = MQTT_PUBLISH_STREAM;
	message->publish.content.data = (uint8_t*)stream;

	bool success = requestQueue.enqueue(message);
	if(success) {
		// Try to force-send message to decrease latency.
		// Should work for small size messages but there is no guarantee.
		onReadyToSendData(TcpConnectionEvent::eTCE_Poll);
	}

	return success;
}

bool MqttClient::subscribe(const String& topic)
{
	debug_d("subscription '%s' registered", topic.c_str());

	if(requestQueue.full()) {
		return false;
	}

	auto message = createMessage(MQTT_TYPE_SUBSCRIBE);

	message->subscribe.topics = (mqtt_topicpair_t*)MQTT_MALLOC(sizeof(mqtt_topicpair_t));
	memset(message->subscribe.topics, 0, sizeof(mqtt_topicpair_t));
	if(!copyString(message->subscribe.topics->name, topic)) {
		delete message;
		return false;
	}

	return requestQueue.enqueue(message);
}

bool MqttClient::unsubscribe(const String& topic)
{
	debug_d("unsubscribing from '%s'", topic.c_str());

	if(requestQueue.full()) {
		return false;
	}

	auto message = createMessage(MQTT_TYPE_SUBSCRIBE);

	message->unsubscribe.topics = (mqtt_topic_t*)MQTT_MALLOC(sizeof(mqtt_topic_t));
	memset(message->unsubscribe.topics, 0, sizeof(mqtt_topic_t));
	if(!copyString(message->unsubscribe.topics->name, topic)) {
		delete message;
		return false;
	}

	return requestQueue.enqueue(message);
}

void MqttClient::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	switch(state) {
	REENTER:
	case eMCS_Ready: {
		if(outgoingMessage != &connectMessage) {
			deleteMessage(outgoingMessage);
		}
		if(connectQueued) {
			outgoingMessage = &connectMessage;
			connectQueued = false;
		} else {
			outgoingMessage = requestQueue.dequeue();
		}
		if(!outgoingMessage) {
			// Send PINGREQ every PingRepeatTime time, if there is no outgoing traffic
			if(!pingTimer.expired()) {
				break;
			}

			outgoingMessage = createMessage(MQTT_TYPE_PINGREQ);
		}

		debug_d("[MQTT] Sending message type %u", outgoingMessage->common.type);

		IDataSourceStream* payloadStream{nullptr};
		if(outgoingMessage->common.type == MQTT_TYPE_PUBLISH &&
		   outgoingMessage->publish.content.length == MQTT_PUBLISH_STREAM) {
			payloadStream = reinterpret_cast<IDataSourceStream*>(outgoingMessage->publish.content.data);
			if(payloadStream) {
				outgoingMessage->publish.content.length = payloadStream->available();
			}
		}

		size_t packetLength = mqtt_serialiser_size(&serialiser, outgoingMessage);
		if(!packetLength) {
			debug_e("Error: Invalid MQTT message detected!");
			break;
		}

		uint8_t packet[packetLength];
		mqtt_serialiser_write(&serialiser, outgoingMessage, packet, packetLength);

		delete stream;
		auto headerStream = new MemoryDataStream();
		headerStream->write(packet, packetLength);
		if(outgoingMessage->common.type == MQTT_TYPE_PUBLISH && payloadStream != nullptr) {
			auto streamChain = new StreamChain();
			streamChain->attachStream(headerStream);
			streamChain->attachStream(payloadStream);
			stream = streamChain;
		} else {
			stream = headerStream;
		}

		state = eMCS_SendingData;
	}

	case eMCS_SendingData:
		pingTimer.start();
		if(stream != nullptr && !stream->isFinished()) {
			break;
		}

		state = eMCS_Ready;
		goto REENTER;

	default:
		break;
	}

	TcpClient::onReadyToSendData(sourceEvent);
}

void MqttClient::onFinished(TcpClientState finishState)
{
	clearBits(flags, MQTT_CLIENT_CONNECTED);
	TcpClient::onFinished(finishState);
}
