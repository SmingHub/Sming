/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "MqttClient.h"

#include "../SmingCore.h"

MqttClient::MqttClient(String serverHost, int serverPort) : TcpClient((bool)false)
{
	server = serverHost;
	port = serverPort;
	waitingSize = 0;
	current = NULL;
}

MqttClient::~MqttClient()
{
}

bool MqttClient::connect(String clientName)
{
	return connect(clientName, "", "");
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
	if (getState() != eTCS_Connected) return false;

	int res = mqtt_publish(&broker, topic.c_str(), message.c_str(), retained);
	return res > 0;
}

bool MqttClient::publishWithQoS(String topic, String message, int QoS, bool retained /* = false*/)
{
	if (getState() != eTCS_Connected) return false;

	int res = mqtt_publish_with_qos(&broker, topic.c_str(), message.c_str(), retained, QoS, NULL);
	return res > 0;
}

int MqttClient::staticSendPacket(void* userInfo, const void* buf, unsigned int count)
{
	MqttClient* client = (MqttClient*)userInfo;
	client->send((const char*)buf, count);
}

void MqttClient::debugPrintResponseType(int type)
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
	default:
		tp = "b" + String(type, 2);
	}
	debugf("> MQTT status: %s", tp.c_str());
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
		if (buf->len < 2)
		{
			// Bad packet?
			debugf("> MQTT WRONG PACKET? (len: %d)", buf->len);
			close();
			return ERR_OK;
		}

		if (waitingSize == 0)
		{
			// It's begining of new packet
			uint8_t* packet = (uint8_t*)buf->payload;
			waitingSize = packet[1] + 2; // Remaining length + fixed header length

			// Prevent overflow
			if (waitingSize < MQTT_MAX_BUFFER_SIZE)
				current = buffer;
			else
				current = NULL;
		}

		if (current != NULL)
		{
			int len = min(buf->tot_len, waitingSize);
			pbuf_copy_partial(buf, current, len, 0);
			current += len;
			waitingSize -= len;

			if (waitingSize == 0)
			{
				// We was taken full packet
				int type = MQTTParseMessageType(buffer);
				debugPrintResponseType(type);
			}
			//TODO: loop next fragments if exist
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
