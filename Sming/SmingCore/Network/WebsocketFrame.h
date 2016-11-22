/*
 * WebsocketFrame.h
 *
 *  Created on: 13 nov. 2016 г.
 *  Author: https://github.com/avr39-ripe - Alexander V, Ribchansky
 *
 *  Some parts of code inspired by: https://github.com/Links2004/arduinoWebSockets and
 *  https://github.com/m8rge/cwebsocket
 */

#ifndef SMINGCORE_NETWORK_WEBSOCKETFRAME_H_
#define SMINGCORE_NETWORK_WEBSOCKETFRAME_H_
#include "../Wiring/WiringFrameworkIncludes.h"


enum class WSFrameType : uint8_t
{
	continuation = 0x00, // continuation frame, UNSUPPORTED BY NOW!
	text = 0x01,		// text frame
	binary = 0x02,		// binary frame
														// %x3-7 are reserved for further non-control frames
	close = 0x08,		// connection close frame
	ping = 0x09,		// ping frame
	pong = 0x0A,		// pong frame
	empty = 0xF0,		// Empty frame, length == 0
	error = 0xF1,		// Error while farsing frame
	incomplete = 0xF2	// Incomplete, inconsistent frame
	///< %xB-F are reserved for further control frame
};

namespace WSFlags
{
    static const uint8_t payloadDeleteMemBit = 1u; //Delete memory reserved for payload in destructor
    static const uint8_t headerDeleteMemBit = 2u; //Delete memory reserved for header in destructor
};

class HttpServer;
class Websocket;
class WebsocketClient;

class WebsocketFrameClass
{
	friend class HttpServer;
	friend class Websocket;
	friend class WebsocketClient;
public:
	WebsocketFrameClass();
	virtual ~WebsocketFrameClass();
	uint8_t encodeFrame(WSFrameType frameType, uint8_t * payload, size_t length, uint8_t mask, uint8_t fin,  uint8_t headerToPayload = true);
	uint8_t decodeFrame(uint8_t * buffer, size_t length);
protected:
	uint8_t* _payload = nullptr;
	size_t _payloadLength = 0;
	uint8_t* _header = nullptr;
	size_t _headerLength = 0;
	WSFrameType _frameType = WSFrameType::empty;
	uint8_t _mask = 0;
	uint8_t _getFrameSizes(uint8_t* buffer, size_t length);
private:
	uint8_t _flags = 0; //Store flags for further freeing memory
	size_t _nextReadOffset = 0; //Store offset in multiframe tcp buffer for next decodeFrame
};

#endif /* SMINGCORE_NETWORK_WEBSOCKETFRAME_H_ */
