/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "MqttClient.h"

#include "../SmingCore.h"

MqttClient::MqttClient(String serverHost, int serverPort, MqttStringSubscriptionCallback callback /* = NULL*/)
	: TcpClient((bool)false)
{
	server = serverHost;
	port = serverPort;
	this->callback = callback;
	waitingSize = 0;
	posHeader = 0;
	current = NULL;
}

MqttClient::~MqttClient()
{
}

bool MqttClient::connect(String clientName)
{
	if (getConnectionState() != eTCS_Ready)
		close();
	return MqttClient::connect(clientName, "", "");
}

bool MqttClient::connect(String clientName, String username, String password)
{
	debugf("MQTT start connection");
	mqtt_init(&broker, clientName.c_str());
	if (clientName.length() > 0)
		mqtt_init_auth(&broker, username.c_str(), password.c_str());

	int keepalive = 20; // Seconds

	TcpClient::connect(server, port);

	mqtt_set_alive(&broker, keepalive);
	broker.socket_info = (void*)this;
	broker.send = staticSendPacket;

	int res = mqtt_connect(&broker);
	setTimeOut(USHRT_MAX);
	return res > 0;
}

bool MqttClient::publish(String topic, String message, bool retained /* = false*/)
{
	int res = mqtt_publish(&broker, topic.c_str(), message.c_str(), retained);
	return res > 0;
}

bool MqttClient::publishWithQoS(String topic, String message, int QoS, bool retained /* = false*/)
{
	int res = mqtt_publish_with_qos(&broker, topic.c_str(), message.c_str(), retained, QoS, NULL);
	return res > 0;
}

int MqttClient::staticSendPacket(void* userInfo, const void* buf, unsigned int count)
{
	MqttClient* client = (MqttClient*)userInfo;
	bool sent = client->send((const char*)buf, count);
	return sent ? count : 0;
}

bool MqttClient::subscribe(String topic)
{
	uint16_t msgId = 0;
	debugf("subscription '%s' registered", topic.c_str());
	int res = mqtt_subscribe(&broker, topic.c_str(), &msgId);
	return res > 0;
}

bool MqttClient::unsubscribe(String topic)
{
	uint16_t msgId = 0;
	debugf("unsubscribing from '%s'", topic.c_str());
	int res = mqtt_unsubscribe(&broker, topic.c_str(), &msgId);
	return res > 0;
}

void MqttClient::debugPrintResponseType(int type, int len)
{
	String tp;
	switch (type)
	{
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
	debugf("> MQTT status: %s (len: %d)", tp.c_str(), len);
}

err_t MqttClient::onReceive(pbuf *buf)
{
	if (buf == NULL)
	{
		// Disconnected, close it
		TcpClient::onReceive(buf);
	}
	else
	{
		if (buf->len < 1)
		{
			// Bad packet?
			debugf("> MQTT WRONG PACKET? (len: %d)", buf->len);
			close();
			return ERR_OK;
		}

		int received = 0;
		while (received < buf->tot_len)
		{
			int type = 0;
			if (waitingSize == 0)
			{
				// It's begining of new packet
				int pos = received;
				if (posHeader == 0)
				{
					//debugf("start posHeader");
					pbuf_copy_partial(buf, &buffer[posHeader], 1, pos);
					pos++;
					posHeader = 1;
				}
				while (posHeader > 0 && pos < buf->tot_len)
				{
					//debugf("add posHeader");
					pbuf_copy_partial(buf, &buffer[posHeader], 1, pos);
					if ((buffer[posHeader] & 128) == 0)
						posHeader = 0; // Remaining Length ended
					else
						posHeader++;
					pos++;
				}

				if (posHeader == 0)
				{
					//debugf("start len calc");
					// Remaining Length field processed
					uint16_t rem_len = mqtt_parse_rem_len(buffer);
					uint8_t rem_len_bytes = mqtt_num_rem_len_bytes(buffer);

					// total packet length = remaining length + byte 1 of fixed header + remaning length part of fixed header
					waitingSize = rem_len + rem_len_bytes + 1;

					type = MQTTParseMessageType(buffer);
					debugPrintResponseType(type, waitingSize);

					// Prevent overflow
					if (waitingSize < MQTT_MAX_BUFFER_SIZE)
					{
						current = buffer;
						buffer[waitingSize] = 0;
					}
					else
						current = NULL;
				}
				else
					continue;
			}

			int available = min(waitingSize, buf->tot_len - received);
			waitingSize -= available;
			if (current != NULL)
			{
				pbuf_copy_partial(buf, current, available, received);
				current += available;

				if (waitingSize == 0)
				{
					// Full packet received
					if(type == MQTT_MSG_PUBLISH)
					{
						const uint8_t *ptrTopic, *ptrMsg;
						uint16_t lenTopic, lenMsg;
						lenTopic = mqtt_parse_pub_topic_ptr(buffer, &ptrTopic);
						lenMsg = mqtt_parse_pub_msg_ptr(buffer, &ptrMsg);
						// Additional check for wrong packet/parsing error
						if (lenTopic + lenMsg < MQTT_MAX_BUFFER_SIZE)
						{
							debugf("%d: %d\n", lenTopic, lenMsg);
							String topic, msg;
							topic.setString((char*)ptrTopic, lenTopic);
							msg.setString((char*)ptrMsg, lenMsg);
							if (callback)
								callback(topic, msg);
						}
						else
						{
							debugf("WRONG SIZES: %d: %d", lenTopic, lenMsg);
						}
					}
				}
			}
			else
				debugf("SKIP: %d (%d)", available, waitingSize + available); // To large!
			received += available;
		}

		// Fire ReadyToSend callback
		TcpClient::onReceive(buf);
	}

	return ERR_OK;
}

void MqttClient::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	if (sleep >= 10)
	{
		mqtt_ping(&broker);
		sleep = 0;
	}
	TcpClient::onReadyToSendData(sourceEvent);
}
