/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TcpConnection.h
 *
 ****/

/** @defgroup tcp TCP
 *  @ingroup networking
 *  @{
 */

#pragma once

#include <Network/Ssl/Session.h>
#include <IpAddress.h>

#define NETWORK_DEBUG

#define NETWORK_SEND_BUFFER_SIZE 1024

enum TcpConnectionEvent {
	eTCE_Connected = 0, ///< Occurs after connection establishment
	eTCE_Received,		///< Occurs on data receive
	eTCE_Sent,			//< Occurs when previous sending was completed
	eTCE_Poll,			//< Occurs on waiting
};

struct pbuf;
class String;
class IDataSourceStream;
class TcpConnection;

typedef Delegate<void(TcpConnection&)> TcpConnectionDestroyedDelegate;

class TcpConnection
{
public:
	TcpConnection(bool autoDestruct) : autoSelfDestruct(autoDestruct)
	{
	}

	TcpConnection(tcp_pcb* connection, bool autoDestruct) : autoSelfDestruct(autoDestruct)
	{
		initialize(connection);
	}

	virtual ~TcpConnection();

public:
	virtual bool connect(const String& server, int port, bool useSsl = false);
	virtual bool connect(IpAddress addr, uint16_t port, bool useSsl = false);
	virtual void close();

	// return -1 on error
	int writeString(const char* data, uint8_t apiflags = TCP_WRITE_FLAG_COPY)
	{
		return write(data, strlen(data), apiflags);
	}

	// return -1 on error
	int writeString(const String& data, uint8_t apiflags = TCP_WRITE_FLAG_COPY)
	{
		return write(data.c_str(), data.length(), apiflags);
	}

	/** @brief Base write operation
	 *  @param data
	 *  @param len
	 *  @param apiflags TCP_WRITE_FLAG_COPY, TCP_WRITE_FLAG_MORE
	 *	@retval int -1 on error
	 */
	virtual int write(const char* data, int len, uint8_t apiflags = TCP_WRITE_FLAG_COPY);

	int write(IDataSourceStream* stream);

	uint16_t getAvailableWriteSize()
	{
		return (canSend && tcp) ? tcp_sndbuf(tcp) : 0;
	}

	void flush();

	void setTimeOut(uint16_t waitTimeOut);

	IpAddress getRemoteIp() const
	{
		return (tcp == nullptr) ? INADDR_NONE : IpAddress(tcp->remote_ip);
	}

	uint16_t getRemotePort() const
	{
		return (tcp == nullptr) ? 0 : tcp->remote_port;
	}

	/**
	 * @brief Sets a callback to be called when the object instance is destroyed
	 * @param destroyedDelegate
	 */
	void setDestroyedDelegate(TcpConnectionDestroyedDelegate destroyedDelegate)
	{
		this->destroyedDelegate = destroyedDelegate;
	}

	/**
	 * @brief Set the SSL session initialisation callback
	 * @param handler
	 */
	void setSslInitHandler(Ssl::Session::InitDelegate handler)
	{
		sslInit = handler;
	}

	// Called by SSL Session when accepting a client connection
	bool setSslConnection(Ssl::Connection* connection)
	{
		if(!sslCreateSession()) {
			return false;
		}
		ssl->setConnection(connection);
		useSsl = true;
		return true;
	}

	/**
	 * @brief Get a pointer to the current SSL session object
	 *
	 * Note that this is typically used so we can query properties of an
	 * established session. If you need to change session parameters this
	 * must be done via `setSslInitHandler`.
	 */
	Ssl::Session* getSsl()
	{
		return ssl;
	}

protected:
	void initialize(tcp_pcb* pcb);
	bool internalConnect(IpAddress addr, uint16_t port);

	bool sslCreateSession();

	/**
	 * @brief Override in inherited classes to perform custom session initialisation
	 *
	 * Called when TCP connection is established before initiating handshake.
	 */
	virtual void sslInitSession(Ssl::Session& session)
	{
		if(sslInit) {
			sslInit(session);
		}
	}

	virtual err_t onConnected(err_t err);
	virtual err_t onReceive(pbuf* buf);
	virtual err_t onSent(uint16_t len);
	virtual err_t onPoll();
	virtual void onError(err_t err);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);

	/*
	 * If there is space in the TCP output buffer, then don't wait for TCP
	 * sent confirmation but try to send more data now
	 * (Invoked from within other TCP callbacks.)
	 */
	void trySend(TcpConnectionEvent event)
	{
		if(tcp != nullptr && getAvailableWriteSize() > 0) {
			onReadyToSendData(event);
		}
	}

	// These methods are called via LWIP handlers
	err_t internalOnConnected(err_t err);
	err_t internalOnReceive(pbuf* p, err_t err);
	err_t internalOnSent(uint16_t len);
	err_t internalOnPoll();
	void internalOnError(err_t err);
	void internalOnDnsResponse(const char* name, LWIP_IP_ADDR_T* ipaddr, int port);

private:
	static err_t staticOnPoll(void* arg, tcp_pcb* tcp);
	static void closeTcpConnection(tcp_pcb* tpcb);

	inline void checkSelfFree()
	{
		if(tcp == nullptr && autoSelfDestruct) {
			delete this;
		}
	}

protected:
	tcp_pcb* tcp = nullptr;
	uint16_t sleep = 0;
	uint16_t timeOut = USHRT_MAX; ///< By default a TCP connection does not have a time out
	bool canSend = true;
	bool autoSelfDestruct = true;
	Ssl::Session* ssl = nullptr;
	Ssl::Session::InitDelegate sslInit;
	bool useSsl = false;

private:
	TcpConnectionDestroyedDelegate destroyedDelegate = nullptr;
};

/** @} */
