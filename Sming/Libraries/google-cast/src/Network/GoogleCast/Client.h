#pragma once

#include <Data/Buffer/CircularBuffer.h>
#include <ArduinoJson.h>
#include "cast_channel.pb.h"

namespace GoogleCast
{
using MessageDelegate = Delegate<bool(extensions_api_cast_channel_CastMessage)>;

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
	Client(size_t storageSize = 1024, bool autoDestruct = false)
		: TcpClient(autoDestruct), inputBuffer(new CircularBuffer(storageSize))
	{
		TcpClient::setReceiveDelegate(TcpClientDataDelegate(&Client::onTcpReceive, this));
	}

	bool connect(const IpAddress addr, int port = 8009);

	/**
	 * @brief  Tries to auto-discover Google Cast enabled TV and connect to it.
	 */
	bool connect();

	void setResponseHandler(MessageDelegate handler)
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
	bool launch(const String& appId);

	bool play(const Url& url, const MimeType& mime)
	{
		return play(url, toString(mime));
	}

	bool play(const Url& url, const String& mime);

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
	bool publish(const String& data);

	/**
	 * @brief Publishes message in the receiver channel
	 */
	bool publish(JsonDocument& json);

	bool ping();

	void close() override;

protected:
	err_t onConnected(err_t err) override;
	err_t onPoll() override;

private:
	String generateMessage(const String& data, ChannelType type, const String& sourceId = nullptr,
						   const String& destinationId = nullptr)
	{
		return generateMessage(reinterpret_cast<const uint8_t*>(data.c_str()), data.length(), type, sourceId,
							   destinationId);
	}

	String generateMessage(const char* data, ChannelType type, const String& sourceId = nullptr,
						   const String& destinationId = nullptr)
	{
		return generateMessage((const uint8_t*)data, strlen(data), type, sourceId, destinationId);
	}

	String generateMessage(const uint8_t* data, size_t length, ChannelType type, const String& sourceId = nullptr,
						   const String& destinationId = nullptr);

	bool onTcpReceive(TcpClient& client, char* data, int length);

private:
	unsigned lastPing{0};
	unsigned pingRepeatTime{4};

	size_t messageLength{0};
	CircularBuffer* inputBuffer;
	MessageDelegate onMessage;
};

} // namespace GoogleCast

/**
 * @brief Get textual representation for a googlecast channel type
 * @param type
 * @retval String
 */
String toString(GoogleCast::ChannelType type);
