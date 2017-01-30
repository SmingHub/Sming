/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef SMINGCORE_NETWORK_WEBSOCKET_H_
#define SMINGCORE_NETWORK_WEBSOCKET_H_

#include "TcpServer.h"
#include "HttpServerConnection.h"
#include "../../Wiring/WHashMap.h"
#include "../../Wiring/WVector.h"
#include "../Delegate.h"
#include "../../Services/cWebsocket/websocket.h"

class HttpServer;
class CommandExecutor;

class WebSocket
{
	friend class HttpServer;
public:
	/** @brief  Object class constructor
	 *  @param conn Pointer to HttpServer connection object that this webSocket will use
	 */
	WebSocket(HttpServerConnection* conn);

	/** @brief  Virtual object class destructor
	 */
	virtual ~WebSocket();

	/** @brief  Send data to other peer
	 *  @param message Pointer to buffer to send
	 *  @param length Length of buffer to send
	 *  @param type Frametype of the message to be sent. Default is text frame.
	 */
	virtual void send(const char* message, int length, wsFrameType type = WS_TEXT_FRAME);

	/** @brief  Send a string to other peer
	 *  @param message String object holding the message to send
	 */
	void sendString(const String& message);

	/** @brief  Send binary data to other peer
	 *  @param data Pointer to buffer to send
	 *  @param size Length of buffer to send
	 */
	void sendBinary(const uint8_t* data, int size);

	/** @brief  Enable command processing
	 */
	void enableCommand();

	/** @brief  Close webSocket connection
	 */
	void close();

	/** @brief  Set TCP connection timeout
	 *  @param  waitTimeOut TCP timeout
	 */
	void setTimeOut(uint16_t waitTimeOut) { if(connection) connection->setTimeOut(waitTimeOut); };

	/** @brief  Test if two sockets are the same
	 *  @param  other The other webSocket
	 *  @return true if the two webSockets are defining the same connection
	 */
	bool operator==(const WebSocket &other) const { return this->connection == other.connection;};

	/** @brief  Store user data pointer for this socket(connection)
	 *  @param  userData Pointer to user defined data
	 */
	void setUserData(void* userData);

	/** @brief  Get user data pointer for this socket(connection)
	 *  @return  Pointer to user defined data
	 */
	void *getUserData();

protected:
	bool initialize(HttpRequest &request, HttpResponse &response);
	bool is(HttpServerConnection* conn) { return connection == conn; };

private:
	HttpServerConnection* connection;
	CommandExecutor* commandExecutor = nullptr;
	void* userData = nullptr;
};

#endif /* SMINGCORE_NETWORK_WEBSOCKET_H_ */
