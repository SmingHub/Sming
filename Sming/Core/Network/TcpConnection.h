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

#include <SslImplementation.h>
#include <IpAddress.h>

#define NETWORK_DEBUG

#define NETWORK_SEND_BUFFER_SIZE 1024

enum TcpConnectionEvent {
	// Occurs after connection establishment
	eTCE_Connected = 0,
	// Occurs on data receive
	eTCE_Received,
	// Occurs when previous sending was completed
	eTCE_Sent,
	// Occurs on waiting
	eTCE_Poll
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
	virtual bool connect(const String& server, int port, bool useSsl = false, uint32_t sslOptions = 0);
	virtual bool connect(IpAddress addr, uint16_t port, bool useSsl = false, uint32_t sslOptions = 0);
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

	void addSslOptions(uint32_t sslOptions)
	{
		this->sslOptions |= sslOptions;
	}

	/**
	 * @brief Sets private key, certificate and password from memory for the SSL connection
	 * 		  If this methods is called from a client then it sets the client key and certificate
	 * 		  If it is called from a server then it sets the server certificate and key.
	 * 		  Server and Client certificates differ. Client certificate is used for identification.
	 * 		  Server certificate is used for encrypt/decrypt the data.
	 * 		  Make sure to use the correct certificate for the desired goal.
	 *
	 * @note  This method makes copy of the data.
	 *
	 * @param key
	 * @param keyLength
	 * @param certificate
	 * @param certificateLength
	 * @param keyPassword
	 * @param freeAfterHandshake
	 *
	 * @return bool  true of success, false or failure
	 */
	bool setSslKeyCert(const uint8_t* key, int keyLength, const uint8_t* certificate, int certificateLength,
					   const char* keyPassword = nullptr, bool freeAfterHandshake = false)
	{
		freeKeyCertAfterHandshake = freeAfterHandshake;
		return sslKeyCert.assign(key, keyLength, certificate, certificateLength, keyPassword);
	}

	/**
	* @brief Sets private key, certificate and password from memory for the SSL connection
	* 	 	 If this methods is called from a client then it sets the client key and certificate
	* 		 If it is called from a server then it sets the server certificate and key.
	* 		 Server and Client certificates differ. Client certificate is used for identification.
	* 		 Server certificate is used for encrypt/decrypt the data.
	* 		 Make sure to use the correct certificate for the desired goal.
	*
	* @note  This method passes the certificate key chain by reference
	*
	* @param keyCert
	* @param freeAfterHandshake
	*
	* @retval bool  true of success, false or failure
	*/
	bool setSslKeyCert(const SslKeyCertPair& keyCert, bool freeAfterHandshake = false)
	{
		freeKeyCertAfterHandshake = freeAfterHandshake;
		return sslKeyCert.assign(keyCert);
	}

	/**
	 * @brief Frees the memory used for the key and certificate pair
	 */
	void freeSslKeyCert()
	{
		sslKeyCert.free();
	}

	// Called by TcpServer
	void setSsl(SslConnection* ssl)
	{
		this->ssl = ssl;
		useSsl = true;
	}

	SslConnection* getSsl()
	{
		return ssl;
	}

protected:
	void initialize(tcp_pcb* pcb);
	bool internalConnect(IpAddress addr, uint16_t port);

	virtual err_t onConnected(err_t err);
	virtual err_t onReceive(pbuf* buf);
	virtual err_t onSent(uint16_t len);
	virtual err_t onPoll();
	virtual void onError(err_t err);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
	virtual err_t onSslConnected(SslConnection* ssl);

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

	SslContext* sslContext = nullptr;
	SslConnection* ssl = nullptr;
	SslExtension* sslExtension = nullptr;
	bool sslConnected = false;
	uint32_t sslOptions = 0;
	SslKeyCertPair sslKeyCert;
	bool freeKeyCertAfterHandshake = false;
	SslSessionId* sslSessionId = nullptr;

	bool useSsl = false;

private:
	TcpConnectionDestroyedDelegate destroyedDelegate = nullptr;

	void closeSsl();
};

/** @} */
