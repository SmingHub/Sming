/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "MqttClient.h"
#include "Clock.h"
#include <algorithm>

MqttClient::MqttClient(bool autoDestruct /* = false*/) : TcpClient(autoDestruct)
{
	mqtt_init(&_broker);
}

// Deprecated . . .
MqttClient::MqttClient(String serverHost, int serverPort, MqttStringSubscriptionCallback callback /* = nullptr*/) :
	TcpClient(false)
{
	_url.setHost(serverHost);
	_url.setPort(serverPort);
	_callback = callback;
	mqtt_init(&_broker);
}

// Deprecated . . .
MqttClient::MqttClient(IPAddress serverIp, int serverPort, MqttStringSubscriptionCallback callback /* = nullptr*/) :
	TcpClient(false)
{
	_url.setHost(serverIp);
	_url.setPort(serverPort);
	_callback = callback;
	mqtt_init(&_broker);
}

MqttClient::~MqttClient()
{
	mqtt_free(&_broker);
}

void MqttClient::setCallback(MqttStringSubscriptionCallback callback)
{
	_callback = callback;
}

void MqttClient::setKeepAlive(int seconds)
{
	_keepAlive = seconds;
}

void MqttClient::setPingRepeatTime(int seconds)
{
	if (_pingRepeatTime > _keepAlive)
		_pingRepeatTime = _keepAlive;
	else
		_pingRepeatTime = seconds;
}

bool MqttClient::setWill(const String& topic, const String& message, int QoS, bool retained /* = false*/)
{
	return mqtt_set_will(&_broker, topic.c_str(), message.c_str(), QoS, retained);
}

bool MqttClient::connect(const URL& url, const String& clientName, uint32_t sslOptions)
{
	bool useSsl;
	if (url.protocol() == _F("mqtts"))
		useSsl = true;
	else if (url.protocol() == _F("mqtt"))
		useSsl = false;
	else {
		debug_e("Only mqtt and mqtts protocols are allowed");
		return false;
	}

	_url = url;
	_waitingSize = 0;
	_posHeader = 0;
	_current = nullptr;

	return privateConnect(clientName, url.user(), url.password(), useSsl, sslOptions);
}

// Deprecated . . .
bool MqttClient::connect(const String& clientName, boolean useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{
	return connect(clientName, nullptr, nullptr, useSsl, sslOptions);
}

// Deprecated . . .
bool MqttClient::connect(const String& clientName, const String& username, const String& password,
						 boolean useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{
	return privateConnect(clientName, username, password, useSsl, sslOptions);
}

bool MqttClient::privateConnect(const String& clientName, const String& username, const String& password,
								boolean useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{
	if (getConnectionState() != eTCS_Ready) {
		close();
		debug_d("MQTT closed previous connection");
	}

	debug_d("MQTT start connection");
	if (clientName.length() > 0) {
		mqtt_set_clientid(&_broker, clientName.c_str());
	}

	if (username.length() > 0) {
		mqtt_init_auth(&_broker, username.c_str(), password.c_str());
	}

	TcpClient::connect(_url.host(), _url.port(), useSsl, sslOptions);

	mqtt_set_alive(&_broker, _keepAlive);
	_broker.socket_info = this;
	_broker.send = staticSendPacket;

	int res = mqtt_connect(&_broker);
	setTimeOut(USHRT_MAX);
	return res > 0;
}

bool MqttClient::publish(String topic, String message, bool retained /* = false*/)
{
	int res = mqtt_publish(&_broker, topic.c_str(), message.c_str(), message.length(), retained);
	return res > 0;
}

bool MqttClient::publishWithQoS(String topic, String message, int QoS, bool retained /* = false*/,
								MqttMessageDeliveredCallback onDelivery /* = nullptr */)
{
	uint16_t msgId = 0;
	int res = mqtt_publish_with_qos(&_broker, topic.c_str(), message.c_str(), message.length(), retained, QoS, &msgId);
	if (QoS == 0 && onDelivery)
		debug_d("The delivery callback is ignored for QoS 0.");
	else if (QoS > 0 && onDelivery && msgId)
		_onDeliveryQueue[msgId] = onDelivery;
	return res > 0;
}

int MqttClient::staticSendPacket(void* userInfo, const void* buf, unsigned count)
{
	MqttClient* client = (MqttClient*)userInfo;
	bool sent = client->send((const char*)buf, count);
	client->_lastMessage = millis();
	return sent ? count : 0;
}

bool MqttClient::subscribe(const String& topic)
{
	uint16_t msgId = 0;
	debug_d("subscription '%s' registered", topic.c_str());
	int res = mqtt_subscribe(&_broker, topic.c_str(), &msgId);
	return res > 0;
}

bool MqttClient::unsubscribe(const String& topic)
{
	uint16_t msgId = 0;
	debug_d("unsubscribing from '%s'", topic.c_str());
	int res = mqtt_unsubscribe(&_broker, topic.c_str(), &msgId);
	return res > 0;
}

void MqttClient::debugPrintResponseType(unsigned type, unsigned len)
{
	String tp;
	switch (type) {
	case MQTT_MSG_CONNACK:
		tp = "MQTT_MSG_CONNACK";
		break;
	case MQTT_MSG_PUBACK:
		tp = "MQTT_MSG_PUBACK";
		break;
	case MQTT_MSG_PUBREC:
		tp = "MQTT_MSG_PUBREC";
		break;
	case MQTT_MSG_PUBREL:
		tp = "MQTT_MSG_PUBREL";
		break;
	case MQTT_MSG_PUBCOMP:
		tp = "MQTT_MSG_PUBCOMP";
		break;
	case MQTT_MSG_SUBACK:
		tp = "MQTT_MSG_SUBACK";
		break;
	case MQTT_MSG_PINGRESP:
		tp = "MQTT_MSG_PINGRESP";
		break;
	case MQTT_MSG_PUBLISH:
		tp = "MQTT_MSG_PUBLISH";
		break;
	default:
		tp = "b" + String(type, 2);
	}
	debug_d("> MQTT status: %s (len: %u)", tp.c_str(), len);
}

err_t MqttClient::onReceive(pbuf* buf)
{
	if (!buf) {
		// Disconnected, close it
		return TcpClient::onReceive(buf);
	}

	if (buf->len < 1) {
		// Bad packet?
		debug_e("> MQTT WRONG PACKET? (len: %d)", buf->len);
		close();
		return ERR_OK;
	}

	unsigned received = 0;
	while (received < buf->tot_len) {
		int type = 0;
		if (_waitingSize == 0) {
			// It's the beginning of a new packet
			int pos = received;
			if (_posHeader == 0) {
				//debug_d("start posHeader");
				pbuf_copy_partial(buf, &_buffer[_posHeader], 1, pos);
				pos++;
				_posHeader = 1;
			}
			while (_posHeader > 0 && pos < buf->tot_len) {
				//debug_d("add posHeader");
				pbuf_copy_partial(buf, &_buffer[_posHeader], 1, pos);
				if ((_buffer[_posHeader] & 0x80) == 0) {
					// Remaining Length ended
					_posHeader = 0;
				}
				else
					_posHeader++;
				pos++;
			}

			if (_posHeader == 0) {
				//debug_d("start len calc");
				// Remaining Length field processed
				uint16_t rem_len = mqtt_parse_rem_len(_buffer);
				uint8_t rem_len_bytes = mqtt_num_rem_len_bytes(_buffer);

				// total packet length = remaining length + byte 1 of fixed header + remaning length part of fixed header
				_waitingSize = rem_len + rem_len_bytes + 1;

				type = MQTTParseMessageType(_buffer);
				debugPrintResponseType(type, _waitingSize);

				// Prevent overflow
				if (_waitingSize < MQTT_MAX_BUFFER_SIZE) {
					_current = _buffer;
					_buffer[_waitingSize] = 0;
				}
				else
					_current = nullptr;
			}
			else
				continue;
		}

		int available = std::min(_waitingSize, buf->tot_len - received);
		_waitingSize -= available;
		if (_current) {
			pbuf_copy_partial(buf, _current, available, received);
			_current += available;

			if (_waitingSize == 0) {
				// Full packet received
				if (type == MQTT_MSG_PUBLISH) {
					const uint8_t *ptrTopic, *ptrMsg;
					uint16_t lenTopic, lenMsg;
					lenTopic = mqtt_parse_pub_topic_ptr(_buffer, &ptrTopic);
					lenMsg = mqtt_parse_pub_msg_ptr(_buffer, &ptrMsg);
					// Additional check for wrong packet/parsing error
					if (lenTopic + lenMsg < MQTT_MAX_BUFFER_SIZE) {
						debug_d("%d: %d\n", lenTopic, lenMsg);
						String topic, msg;
						topic.setString((char*)ptrTopic, lenTopic);
						msg.setString((char*)ptrMsg, lenMsg);
						if (_callback) {
							_callback(topic, msg);
						}
					}
					else {
						debug_e("WRONG SIZES: %d: %d", lenTopic, lenMsg);
					}
				}
				else if (type == MQTT_MSG_PUBACK || type == MQTT_MSG_PUBREC) {
					// message with QoS 1 or 2 was received and this is the confirmation
					const uint16_t msgId = mqtt_parse_msg_id(_buffer);
					debug_d("message with id: %d was delivered", msgId);
					if (_onDeliveryQueue.contains(msgId)) {
						// there is a callback for this message
						_onDeliveryQueue[msgId](msgId, type);
						_onDeliveryQueue.remove(msgId);
					}
				}
			}
		}
		else {
			debug_d("SKIP: %d (%d)", available, _waitingSize + available); // Too large!
		}
		received += available;
	}

	// Fire ReadyToSend callback
	TcpClient::onReceive(buf);

	return ERR_OK;
}

void MqttClient::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	// Send PINGREQ every PingRepeatTime time, if there is no outgoing traffic
	// PingRepeatTime should be <= keepAlive
	if (_lastMessage && (millis() - _lastMessage >= _pingRepeatTime * 1000))
		mqtt_ping(&_broker);

	TcpClient::onReadyToSendData(sourceEvent);
}
