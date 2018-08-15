/*
 * WebsocketFrame.h
 *
 *  Created on: 13 nov. 2016 ?.
 *  Author: https://github.com/avr39-ripe - Alexander V, Ribchansky
 *
 *  Some parts of code inspired by: https://github.com/Links2004/arduinoWebSockets and
 *  https://github.com/m8rge/cwebsocket
 */

#include "WebsocketFrame.h"

WebsocketFrameClass::~WebsocketFrameClass()
{
	if (_flags & WSFlags::payloadDeleteMemBit)
		delete[] _payload;

	if (_flags & WSFlags::headerDeleteMemBit)
		delete[] _header;
}

bool WebsocketFrameClass::encodeFrame(WSFrameType opcode, uint8_t* payload, size_t length, uint8_t mask, uint8_t fin,
									  uint8_t headerToPayload)
{
	if (length == 0 || length > 0xFFFF) {
		// either zero length or too big for poor esp8266
		return false;
	}

	uint8_t* headerPtr = nullptr;
	uint8_t* dataPtr = nullptr;

	_payload = payload;
	_payloadLength = length;

	//calculate header size
	length < 126 ? _headerLength = 2 : _headerLength = 4;
	if (mask) {
		// if present, mask is 4 bytes in header before payload
		_headerLength += 4;
	}

	//try to allocate single buffer for both frame header and frame payload
	if (headerToPayload && (length < 1400) && (system_get_free_heap_size() > 6000)) {
		_payloadLength = length + _headerLength;
		dataPtr = new uint8_t[_payloadLength];
		if (dataPtr) {
			_flags |= WSFlags::payloadDeleteMemBit;
			os_memcpy((dataPtr + _headerLength), payload,
					  length); //copy original data to newly created buffer with _headerLength offset
			_payload = dataPtr;
		}
		else {
			// memory allocation failed for single buffer, continue in light-memory mode with separate header and original payload
			headerToPayload = false;
		}
	}

	// set Header Pointer
	if (headerToPayload) {
		// Header is inside _payload buffer and occupy first _headerLength bytes
		_header = _payload;
	}
	else {
		headerPtr = new uint8_t[_headerLength];
		if (!headerPtr)
			return false;

		_header = headerPtr;
		_flags |= WSFlags::headerDeleteMemBit;
	}

	memset(_header, 0, _headerLength); //set initial header state to be all zero

	// create header

	// byte 0
	if (fin) {
		// set FIN
		*_header |= bit(7);
	}

	// set opcode
	*_header |= (uint8_t)opcode;
	_header++;

	// byte 1
	if (mask) {
		// set mask
		*_header |= bit(7);
	}

	if (length < 126) {
		*_header |= length;
		_header++;
	}
	else if (length < 0xFFFF) {
		*_header |= 126;
		_header++;
		*_header = ((length >> 8) & 0xFF);
		_header++;
		*_header = (length & 0xFF);
		_header++;
	}

	if (mask && headerToPayload) {
		// we work on copy of original data so we can mask it without affecting original
		union __attribute__((packed)) {
			uint32_t u32;
			uint8_t b[4];
		} mask;
		mask.u32 = os_random();

		*(uint32_t*)_header = mask.u32;
		_header += 4;

		uint8_t* dataMaskPtr = (_payload + _headerLength);

		for (size_t x = 0; x < length; x++)
			dataMaskPtr[x] ^= mask.b[x % 4];
	}

	if (headerToPayload) {
		// Indicate to caller that whole frame is referenced by _payload
		_header = nullptr;
		_headerLength = 0;
	}
	else
		_header = headerPtr;

	return true;
}

bool WebsocketFrameClass::getFrameSizes(uint8_t* buffer, size_t length)
{
	uint16_t payloadLength = buffer[1] & 0b01111111; // length of payload
	_mask = buffer[1] & 0b10000000;					 // extracting Mask bit

	//calculate header size
	payloadLength < 126 ? _headerLength = 2 : _headerLength = 4;

	if (_mask)
		_headerLength += 4;

	if (payloadLength == 126 && length < 4) {
		_frameType = WSFrameType::incomplete;
		return false;
	}

	if (payloadLength == 127) {
		// unsupported frame size!
		_frameType = WSFrameType::error;
		return false;
	}

	if (payloadLength == 126) {
		// next 2 bytes are length
		payloadLength = (buffer[2] << 8) | buffer[3];
	}

	_payloadLength = payloadLength;

	// if given buffer is multiframe buffer store read offset for next frame
	_nextReadOffset += _headerLength + _payloadLength;

	if (length < _nextReadOffset) {
		// Frame is incomplete
		_frameType = WSFrameType::incomplete;
		return false;
	}

	// check if given buffer is single frame buffer
	if (length == _nextReadOffset) {
		// single websocket frame in buffer
		_nextReadOffset = 0;
	}

	return true;
}

bool WebsocketFrameClass::decodeFrame(uint8_t* buffer, size_t length)
{
	/*
	 * if called again for multiframe buffer, rewind buffer first to next frame
	 * no effect for single frame buffer as initial value of _nextReadOffset = 0
	 */
	buffer += _nextReadOffset;

	// Extract Opcode and FIN bit (single frame)
	WSFrameType op = (WSFrameType)(buffer[0] & 0b00001111);
	uint8_t fin = buffer[0] & 0b10000000;

	// At least there must be one byte that op and fin are valid
	if (length == 0)
		return true;

	if (op != WSFrameType::continuation && op != WSFrameType::text && op != WSFrameType::binary)
		return true;

	if (!fin)
		return true;

	// get _headerLength, _payloadLength and _mask
	bool result = getFrameSizes(buffer, length);

	if (!result) {
		_payload = nullptr;
		_payloadLength = 0;
		_header = nullptr;
		_headerLength = 0;
		_nextReadOffset = 0;
		return false;
	}

	_header = &buffer[0];
	_payload = &buffer[_headerLength];
	_frameType = op;

	if (_mask) {
		const uint8_t* mask = &buffer[_headerLength - 4];
		for (uint8_t i = 0; i < _payloadLength; i++)
			_payload[i] ^= mask[i % 4];
	}

	return true;
}

void WebsocketFrameClass::mask(const String& payload, uint32_t key, char* data)
{
	key = htonl(key);
	for (unsigned i = 0; i < payload.length(); i++)
		data[i] = (payload[i] ^ reinterpret_cast<const uint8_t*>(key)[i % 4]);
}
