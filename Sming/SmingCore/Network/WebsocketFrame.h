/*
 * WebsocketFrame.h
 *
 *  Created on: 13 nov. 2016 ?.
 *  Author: https://github.com/avr39-ripe - Alexander V, Ribchansky
 *
 *  Some parts of code inspired by: https://github.com/Links2004/arduinoWebSockets and
 *  https://github.com/m8rge/cwebsocket
 */

/**	@defgroup wsframe Websocket Frame
 *	@brief	Encode / Decode websocket frames
 *  @{
 */
#ifndef SMINGCORE_NETWORK_WEBSOCKETFRAME_H_
#define SMINGCORE_NETWORK_WEBSOCKETFRAME_H_
#include "../Wiring/WiringFrameworkIncludes.h"

/** @brief  Websocket frame types enum
 */
enum class WSFrameType : uint8_t {
	continuation = 0x00, // continuation frame, UNSUPPORTED BY NOW!
	text = 0x01,		 // text frame
	binary = 0x02,		 // binary frame
						 // %x3-7 are reserved for further non-control frames
	close = 0x08,		 // connection close frame
	ping = 0x09,		 // ping frame
	pong = 0x0A,		 // pong frame
	empty = 0xF0,		 // Empty frame, length == 0. Not from RFC6455! For internal class usage!
	error = 0xF1,		 // Error while farsing frame. Not from RFC6455! For internal class usage!
	incomplete = 0xF2	// Incomplete, inconsistent frame. Not from RFC6455! For internal class usage!
						 ///< %xB-F are reserved for further control frame
};

namespace WSFlags {
static const uint8_t payloadDeleteMemBit = 1u; //Delete memory reserved for payload in destructor
static const uint8_t headerDeleteMemBit = 2u;  //Delete memory reserved for header in destructor
};											   // namespace WSFlags

// Declare classes where WebsocketFrameClass can be used they will have access to _payload and _header members
class HttpServer;
class Websocket;
class WebsocketClient;

/** @brief Websocket Frame
*/
class WebsocketFrameClass {
	friend class HttpServer;
	friend class Websocket;
	friend class WebsocketClient;

public:
	WebsocketFrameClass(){};
	virtual ~WebsocketFrameClass();

	/** @brief  Encode given buffer to valid websocket frame
	 *  @param  frameType Websocket frame type text or binary
	 *  @param  payload Pointer to buffer to be encoded as websocket frame
	 *  @param	length Length of buffer to be encoded as websocket frame
	 *  @param	mask If true websocket frame will be masked (required for client->server communication)
	 *  @param	fin If true produce ordinary websocket frame, not continuation. Currently MUST be true.
	 *  @param	headerToPayload If true try to create single buffer message with header and payload, otherwise produce separate header and payload buffers
	 *  @retval Return true on success, false on error
	 *
	 *  @details if successfully executed, check whether _header is not nullptr and either use _header and _payload or just _payload as websocket frame
	 *
	 */
	uint8_t encodeFrame(WSFrameType frameType, uint8_t* payload, size_t length, uint8_t mask, uint8_t fin,
						uint8_t headerToPayload = true);

	/** @brief  Decode given buffer containing websocket frame to payload
	 *  @param  buffer Pointer to buffer to be decoded as websocket frame
	 *  @param	length Length of buffer to be decoded as websocket frame
	 *  @retval Return true on success, false on error
	 *
	 *  @details if successfully executed, check _frameType to decide what to do with payload pointed by _payload
	 *
	 */
	uint8_t decodeFrame(uint8_t* buffer, size_t length);

	static int mask(const String& payload, uint32_t key, char* data);

protected:
	uint8_t* _payload =
		nullptr; // pointer to payload; in encode - will point to proper websocket frame payload, in decode - will point to websocket frame's decoded data
	size_t _payloadLength = 0;
	uint8_t* _header = nullptr; // pointer to header; in encode - will point to websocket frame header
	size_t _headerLength = 0;
	WSFrameType _frameType = WSFrameType::empty;
	uint8_t _mask = 0;
	uint8_t _getFrameSizes(uint8_t* buffer, size_t length); // used to get frame size from websocket buffer
private:
	uint8_t _flags = 0;			//Store flags for further freeing memory
	size_t _nextReadOffset = 0; //Store offset in multiframe tcp buffer for next decodeFrame
};
/** @} */
#endif /* SMINGCORE_NETWORK_WEBSOCKETFRAME_H_ */
