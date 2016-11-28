/* 
 * File:   WebsocketClient.cpp
 * Author: https://github.com/hrsavla
 *
 * Created on August 4, 2015, 1:37 PM
 * This Websocket Client library is ported by me into Sming from  
 * https://github.com/MORA99/Stokerbot/tree/master/Libraries/WebSocketClient 
 * 
 * 
 */

#include "WebsocketClient.h"

WebsocketClient::WebsocketClient(bool autoDestruct /*= false*/) :
		TcpClient(autoDestruct)
{
	this->Mode = ws_Disconnected;
	this->rxcallback = NULL;
	this->completecallback = NULL;
	this->connectedcallback = NULL;
}

WebsocketClient::~WebsocketClient()
{

}


/* Function Name: setOnReceiveCallback
 * Description: This function is used to set Receive callback function
 * Parameters: Callback function like wsMessageReceived(String message)
 */
void WebsocketClient::setOnReceiveCallback(WebSocketRxCallback _rxcallback)
{
    this->rxcallback = _rxcallback;
}

/* Function Name: setOnDisconnectedCallback
 * Description: This function is used to set Disconnected callback function
 * Parameters: Callback function like  wsDisconnected(bool success)
 */
void WebsocketClient::setOnDisconnectedCallback(WebSocketCompleteCallback _completecallback)
{
	this->completecallback =_completecallback;
}


/* Function Name: setOnConnectedCallback
 * Description: This function is used to set Connected callback function
 * Parameters: Callback function like connectedCallback(wsMode  Mode)
 */
void WebsocketClient::setOnConnectedCallback(WebSocketConnectedCallback _connectedcallback)
{
	this->connectedcallback = _connectedcallback;
}



/* Function Name: connect
 * Description: This function is called to start Websocket Client connection to Server
 * Parameters: url  - Url of server in String like echo.websocket.org
 * 			  WebsocketRxCallback  - Callback function for recieved messages like wsMessageReceived(String message)
 * 			  WebSocketCompleteCallback - Callback function which lets user know
 * 			  							websocket client connection is closed.
 * 			  							eg. wsDisconnected(bool success)
 *
 */
bool WebsocketClient::connect(String url)
{
	this->uri = URL(url);
	this->_url = url;
	TcpClient::connect(uri.Host,uri.Port);
	debugf("Connecting to Server");
	unsigned char keyStart[17];
	char b64Key[25];
	Mode = ws_Connecting; // Server Connected / WS Upgrade request sent

	randomSeed(analogRead(0));

	for (int i = 0; i < 16; ++i)
	{
		keyStart[i] = (char) random(1, 256);
	}

	base64_encode(16, (const unsigned char*) keyStart, 24, (char*) b64Key);

	for (int i = 0; i < 24; ++i)
	{
		key[i] = b64Key[i];
	}
	String protocol = "chat";
	sendString("GET ");
	if (uri.Path != "")
	{
		sendString(uri.Path);
	}
	else
	{
		sendString("/");
	}
	sendString(" HTTP/1.1\r\n");
	sendString("Upgrade: websocket\r\n");
	sendString("Connection: Upgrade\r\n");
	sendString("Host: ");
	sendString(uri.Host);
	sendString("\r\n");
	sendString("Sec-WebSocket-Key: ");
	sendString(key);
	sendString("\r\n");
	sendString("Sec-WebSocket-Protocol: ");
	sendString(protocol);
	sendString("\r\n");
	sendString("Sec-WebSocket-Version: 13\r\n");
	sendString("\r\n", false);
	return true;

}

/* Function Name: onConnected
 * Description: This function is called when Websocket Client is Connected to Server
 * Parameters: err - err_t state
 * 			if err = ERR_OK (0) then connection is successful. You can then send data.
 */
err_t WebsocketClient::onConnected(err_t err)
{
	if (err == ERR_OK)
	{
		//Mode = ws_Connecting;
		//	debugf("***OnConnected CALLED**** - %d", err); // No longer needed

	}

	TcpClient::onConnected(err);
}

/* Function Name: onError
 * Description: This function is called when Websocket Client is Connected to Server
 * Parameters: err - err_t state
 * 			Standard error codes and their meaning#define
 * 			ERR_OK          0    No error, everything OK.
 * 			ERR_MEM        -1    Out of memory error.
 *    		ERR_BUF        -2    Buffer error.
 * 			ERR_TIMEOUT    -3    Timeout.
 *  		ERR_RTE        -4    Routing problem.
 *			ERR_INPROGRESS -5    Operation in progress
 *			ERR_VAL        -6    Illegal value.
 *			ERR_WOULDBLOCK -7    Operation would block.
 *			ERR_ABRT       -8    Connection aborted.       ***Common Cause
 *			ERR_RST        -9    Connection reset.        *** Common Cause
 *			ERR_CLSD       -10   Connection closed.
 *			ERR_CONN       -11   Not connected.
 *
 */
void WebsocketClient::onError(err_t err)
{

	if ((err == ERR_ABRT) || (err == ERR_RST))
	{
		debugf("TCP Connection Reseted or Aborted", err);

	}
	else
	{
		debugf("Error  %d Occured ", err);
	}
	TcpClient::onError(err);

}

/* Function Name: verifyKey
 * Description: It verifies Encrypted key sent by server
 * Parameters: buf  -  buffer received 
 *             size - size of buffer             
 */
bool WebsocketClient::verifyKey(char* buf, int size)
{
	String dd = String(buf);
	uint8_t s = dd.indexOf("Sec-WebSocket-Accept: ");
	uint8_t t = dd.indexOf("\r\n", s);
	String serverKey = dd.substring(s + 22, t);
	//debugf("ServerKey : %s",serverKey.c_str());
	String hash = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	unsigned char data[20];
	char secure[20 * 4];
	sha1(data, hash.c_str(), hash.length());
	base64_encode(20, data, 20 * 4, secure);

	//debugf("clienKey : %s",secure);
	// if the keys match, good to go
	return serverKey.equals(String(secure)); //b64Result
}

/* Function Name: onFinished
 * Description: This function is called when Websocket Client is Closed
 * Parameters: finishState - State of client             
 */
void WebsocketClient::onFinished(TcpClientState finishState)
{
	Mode = ws_Disconnected;
	if (finishState == eTCS_Failed)
	{
		//  restart();
		debugf("Tcp Client failure...");
		this->completecallback(false);
	}
	else
	{
		debugf("Websocket Closed Normally.");
		this->completecallback(true);
	}
	TcpClient::onFinished(finishState);
}



/* Function Name: sendPing
 * Description: Sending Ping packet to verify Server is connected
 * 				Ping can also be send to keep connection alive.
 * Parameters:       
 */
void WebsocketClient::sendPing()
{
	uint8_t buf[2] =
	{ 0x89, 0x00 };
	debugf("Sending PING");
	send((char*) buf, 2, false);
}

/* Function Name: sendPong
 * Description: Pong is send in response to Ping sent by Server
 * Parameters:       
 */
void WebsocketClient::sendPong()
{
	uint8_t buf[2] =
	{ 0x8A, 0x00 };
	debugf("Sending PONG");
	send((char*) buf, 2, false);
}

/* Function Name: disconnect
 * Description: Close Websocket Client cleanly.
 * Parameters:       
 */
void WebsocketClient::disconnect()
{
	debugf("Terminating Websocket connection.");
	Mode = ws_Disconnected;
	// Should send 0x87, 0x00 to server to tell it that I'm quitting here.
	uint8_t buf[2] =
	{ 0x87, 0x00 };
	send((char*) buf, 2, true);
	//TcpClient::flush();
	//TcpClient::close();
}


/* Function Name: sendMessage
 * Description: Send Text message to Websocket Server
 *              Max length of message permitted is 0xffff (65535) bytes
 * Parameters: msg - Message char array 
 *             length - length of msg char array
 */
void WebsocketClient::sendMessage(char* msg, uint16_t length)
{
	/*
	 +-+-+-+-+-------+-+-------------+-------------------------------+
	 0                   1                   2                   3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	 +-+-+-+-+-------+-+-------------+-------------------------------+
	 |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
	 |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
	 |N|V|V|V|       |S|             |   (if payload len==126/127)   |
	 | |1|2|3|       |K|             |                               |
	 +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
	 |     Extended payload length continued, if payload len == 127  |
	 + - - - - - - - - - - - - - - - +-------------------------------+
	 |                               | Masking-key, if MASK set to 1 |
	 +-------------------------------+-------------------------------+
	 | Masking-key (continued)       |          Payload Data         |
	 +-------------------------------- - - - - - - - - - - - - - - - +
	 :                     Payload Data continued ...                :
	 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
	 |                     Payload Data continued ...                |
	 +---------------------------------------------------------------+

	 Opcodes
	 0x00: this frame continues the payload from the last.
	 0x01: this frame includes utf-8 text data.
	 0x02: this frame includes binary data.
	 0x08: this frame terminates the connection.
	 0x09: this frame is a ping.
	 0x0A: this frame is a pong.
	 */
	uint8_t buffer[500];
	uint8_t i = 0;
	buffer[i++] = 0x80 | 0x01; //FINN = 1 and Sending Text Data

	/*
	 payload_len (7 bits): the length of the payload.

	 0-125 mean the payload is that long.
	 126 means that the following two bytes indicate the length.
	 127 means the next 8 bytes indicate the length.

	 So it comes in ~7bit, 16bit and 64bit.
	 */

	if (length <= 125)
	{
		buffer[i++] = 0x80 | length; //Setting MASK bit and length
	}
	else
	{
		buffer[i++] = 0xFE; // Mask bit + 126
		buffer[i++] = (uint8_t) (length >> 8);
		buffer[i++] = (uint8_t) (length & 0xFF);
	}
	//64bit outgoing messenges not supported

	byte mask[4];
	for (uint8_t j = 0; j < 4; j++)
	{
		mask[j] = random(0, 255);
		buffer[i++] = mask[j];
	}
	for (uint16_t j = 0; j < length; j++)
	{
		buffer[i++] = (msg[j] ^ mask[j % 4]);
	}
	send((char*) buffer, i, false);
}

/* Function Name: sendBinary
 * Description: Send Binary message to Websocket Server
 *              Max length of message permitted is 0xffff (65535) bytes
 * Parameters: msg - Message int8 array 
 *             length - length of msg  array
 */
void WebsocketClient::sendBinary(uint8_t* msg, uint16_t length)
{
	uint8_t buffer[500];
	uint8_t i = 0;
	buffer[i++] = 0x80 | 0x02; //FINN = 1 and Sending Binary Data

	if (length <= 125)
	{
		buffer[i++] = 0x80 | length; //Setting MASK bit and length
	}
	else
	{
		buffer[i++] = 0xFE; // Mask bit + 126
		buffer[i++] = (uint8_t) (length >> 8);
		buffer[i++] = (uint8_t) (length & 0xFF);
	}
	//64bit outgoing messenges not supported

	byte mask[4];
	for (uint8_t j = 0; j < 4; j++)
	{
		mask[j] = random(0, 255);
		buffer[i++] = mask[j];
	}
	for (uint16_t j = 0; j < length; j++)
	{
		buffer[i++] = (msg[j] ^ mask[j % 4]);
	}
	send((char*) buffer, i, false); // Need support from TcpClient to send binary
}

/* Function Name: onReceive
 * Description: It process message sent byWebsocket Server
 *              Max length of message permitted is 0xffff (65535) bytes
 * Parameters: buf - pbuf array 
 */
err_t WebsocketClient::onReceive(pbuf* buf)
{
	if (buf == NULL)
	{
		// Disconnected, close it
		TcpClient::onReceive(buf);
	}
	else
	{
		uint16_t size = buf->tot_len;
		char* data = new char[size + 1];
		pbuf_copy_partial(buf, data, size, 0);
		data[size] = '\0';

		//  debugf("%s", data); //print received buffer
		switch (Mode)
		{
		case ws_Connecting:
			if (verifyKey(data, size) == true)
			{
				Mode = ws_Connected;
				this->connectedcallback(Mode);
				//   debugf("Key Verified. Websocket Handshake completed");
				sendPing();
			}
			else
			{
				Mode = ws_Disconnected; // Handshake was not proper.
				this->connectedcallback(Mode);
			}
			break;

		case ws_Connected:
			// Parsing received Websocket packet
			uint8_t op = data[0] & 0b00001111; // Extracting Opcode
			uint8_t fin = data[0] & 0b10000000; // Extracting Fin Bit (Single Frame)
			// debugf("Opcode : 0x%x",op);
			// debugf("Fin : 0x%x",fin);
			if (op == 0x00 || op == 0x01 || op == 0x02) //Data
			{
				if (fin > 0)
				{
					//  debugf("Single frame message");
					char masked = data[1] & 0b10000000; // extracting Mask bit
					uint16_t len = data[1] & 0b01111111; // length of data
					uint16_t cnt = 2;
					if (len == 126)
					{
						//next 2 bytes are length
						len = data[cnt++];
						len << 8;
						len = len | data[cnt++];
					}
					if (len == 127)
					{
						//next 8 bytes are length
						debugf("64bit messenges not supported"); // Too big for Esp8266 to handle
						return -1;
					}
					// debugf("Message is %d chars long",len);

					//Generally server replies are not masked, but RFC does not forbid it
					uint8_t mask[4];
					if (masked > 0)
					{
						mask[0] = data[cnt++];
						mask[1] = data[cnt++];
						mask[2] = data[cnt++];
						mask[3] = data[cnt++];
					}

					uint8_t rxdata[len + 1]; //Max 16bit length message, so 65kbyte ...
					for (uint8_t i = 0; i < len; i++)
					{
						rxdata[i] = data[cnt++];
						if (masked > 0)
							rxdata[i] = rxdata[i] ^ mask[i % 4];
					}
					rxdata[len] = '\0';
					// debugf("Frame Contents :");
					//   debugf("%s",(char* )rxdata);
					//This is UTF-8 code, but for the general ASCII table UTF8 and ASCII are the same, so it wont matter if we dont send/recieve special chars.
					this->rxcallback((char*) rxdata); //send data to callback function;
				} //Currently this code does not handle fragmented messenges, since a single message can be 64bit long, only streaming binary data seems likely to need fragmentation.

			}
			else if (op == 0x08)
			{
				debugf("Got Disconnect request from server.");
				//RFC requires we return a close op code before closing the connection
				disconnect();
			}
			else if (op == 0x09)
			{
				debugf("Got ping ...");
				sendPong(); //Need to send Pong in response to Ping
			}
			else if (op = 0x10)
			{
				debugf("Got pong ...");
				//A pong can contain app data, but shouldnt if we didnt send any...

			}
			else
			{
				debugf("Unknown opcode : %d ", op);
				//Or not start of package if we failed to parse the entire previous one
			}
			break;
		}
		TcpClient::onReceive(buf);
	}
}

/* Function Name: sendMessage
 * Description: Send Text Stringmessage to Websocket Server
 *              Max length of message permitted is 0xffff (65535) bytes
 * Parameters: Str - String to be sent to websocket server
 */
void WebsocketClient::sendMessage(String str)
{
	uint16_t size = str.length() + 1;
	char cstr[size];

	str.toCharArray(cstr, size);
	sendMessage(cstr, size);
}
/* Function Name: getWSMode
 * Description: Gets present Mode of Websocet client
 *              ws_Disconnected = 0  (Websocket Client is disconnected)
 *              ws_Connecting =1 (Websocket Client is connected Server)
 *              ws_Connected = 2 (Websocket is connected to server and can send
 *                                 and receive messages.)
 */
wsMode WebsocketClient::getWSMode()
{
	return Mode;
}
