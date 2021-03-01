/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Client.h
 *
 ****/

#pragma once

#include "Connection.h"
#include "Heartbeat.h"
#include "Receiver.h"
#include "Media.h"

class LimitedMemoryStream;

namespace GoogleCast
{
class Client : protected TcpClient
{
public:
	using ConnectDelegate = Delegate<void(bool success)>;
	using MessageDelegate = Delegate<bool(Channel::Message& message)>;

	Client(bool autoDestruct = false)
		: TcpClient(autoDestruct), connection(*this), heartbeat(*this), receiver(*this), media(*this)
	{
		TcpClient::setReceiveDelegate(TcpClientDataDelegate(&Client::onTcpReceive, this));
		pingTimer.reset(4);
	}

	bool connect(const IpAddress addr, int port = 8009);

	/**
	 * @brief  Tries to auto-discover Google Cast enabled TV and connect to it.
	 */
	bool connect();

	void onConnect(ConnectDelegate handler)
	{
		connectCallback = handler;
	}

	void onMessage(MessageDelegate handler)
	{
		messageCallback = handler;
	}

	/**
	 * Sets the interval in which to ping the remote server if there was no activity
	 * @param seconds
	 */
	void setPingRepeatTime(unsigned seconds)
	{
		pingTimer.reset(seconds);
	}

	/** @} */

	bool ping();

	void close() override;

private:
	// Must initialise `channels` first because it's accessed via Channel constructor
	Channel::List channels;

public:
	Connection connection;
	Heartbeat heartbeat;
	Receiver receiver;
	Media media;

protected:
	err_t onConnected(err_t err) override;
	void onError(err_t err) override;
	void onClosed() override;
	err_t onPoll() override;

private:
	friend class Channel;
	bool sendMessage(const Channel& channel, const void* data, size_t length)
	{
		return sendMessage(data, length, channel.getNameSpace(), channel.getSourceId(), channel.getDestinationId());
	}

	bool sendMessage(const void* data, size_t length, const String& nameSpace, const String& sourceId = nullptr,
					 const String& destinationId = nullptr);

	bool onTcpReceive(TcpClient& client, char* data, int length);

	bool dispatch(Channel::Message& message);

	OneShotElapseTimer<NanoTime::Seconds> pingTimer;
	LimitedMemoryStream* inputBuffer{nullptr};
	ConnectDelegate connectCallback;
	MessageDelegate messageCallback;
};

} // namespace GoogleCast
