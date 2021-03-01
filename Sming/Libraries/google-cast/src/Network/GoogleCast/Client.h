#pragma once

#include <Data/Stream/LimitedMemoryStream.h>
#include <ArduinoJson.h>
#include "Messages.h"

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

enum class ChannelType {
#define XX(type, ns) type,
	CAST_CHANNEL_TYPE_MAP(XX)
#undef XX
};

class Client : protected TcpClient
{
public:
	Client(bool autoDestruct = false) : TcpClient(autoDestruct)
	{
		TcpClient::setReceiveDelegate(TcpClientDataDelegate(&Client::onTcpReceive, this));
		pingTimer.reset(4);
	}

	bool connect(const IpAddress addr, int port = 8009);

	/**
	 * @brief  Tries to auto-discover Google Cast enabled TV and connect to it.
	 */
	bool connect();

	void setResponseHandler(ChannelMessage::Delegate handler)
	{
		onMessage = handler;
	}

	/**
	 * Sets the interval in which to ping the remote server if there was no activity
	 * @param seconds
	 */
	void setPingRepeatTime(unsigned seconds)
	{
		pingTimer.reset(seconds);
	}

	/* High Level Commands */
	bool launch(const String& appId);

	bool load(const Url& url, MimeType mime)
	{
		return load(url, toString(mime));
	}

	bool load(const Url& url, const String& mime);

	bool pause(const String& sessionId);
	bool play(const String& sessionId);
	bool stop(const String& sessionId);

	bool getStatus();

	bool isAppAvailable(Vector<String> appIds);

	/**
	 * @brief: Sets volume.
	 * @param level is a float between 0 and 1
	 *
	 * @return bool
	 */
	bool setVolumeLevel(float level);

	bool setVolumeMuted(bool muted);

	/* Low Level Commands */

	/**
	 * @brief Publishes message in the receiver channel
	 */
	bool publish(const String& data)
	{
		return sendMessage(data, ChannelType::RECEIVER);
	}

	/**
	 * @brief Publishes message in the receiver channel
	 */
	bool publish(JsonDocument& json)
	{
		return publish(Json::serialize(json));
	}

	bool ping();

	void close() override;

protected:
	err_t onConnected(err_t err) override;
	err_t onPoll() override;

private:
	bool sendMessage(const String& data, ChannelType type, const String& sourceId = nullptr,
					 const String& destinationId = nullptr)
	{
		return sendMessage(reinterpret_cast<const uint8_t*>(data.c_str()), data.length(), type, sourceId,
						   destinationId);
	}

	bool sendMessage(const char* data, ChannelType type, const String& sourceId = nullptr,
					 const String& destinationId = nullptr)
	{
		return sendMessage(data, strlen(data), type, sourceId, destinationId);
	}

	bool sendMessage(const void* data, size_t length, ChannelType type, const String& sourceId = nullptr,
					 const String& destinationId = nullptr);

	bool onTcpReceive(TcpClient& client, char* data, int length);

	OneShotElapseTimer<NanoTime::Seconds> pingTimer;
	size_t messageLength{0};
	LimitedMemoryStream* inputBuffer{nullptr};
	ChannelMessage::Delegate onMessage;
};

} // namespace GoogleCast

/**
 * @brief Get textual representation for a googlecast channel type
 * @param type
 * @retval String
 */
String toString(GoogleCast::ChannelType type);
