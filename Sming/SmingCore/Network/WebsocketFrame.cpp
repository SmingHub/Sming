/*
 * WebsocketFrame.h
 *
 *  Created on: 13 nov. 2016 г.
 *  Author: https://github.com/avr39-ripe - Alexander V, Ribchansky
 *
 *  Some parts of code inspired by: https://github.com/Links2004/arduinoWebSockets and
 *  https://github.com/m8rge/cwebsocket
 */

#include <Network/WebsocketFrame.h>

WebsocketFrameClass::~WebsocketFrameClass()
{
	if((_flags & WSFlags::payloadDeleteMemBit) != 0) {
		delete[] _payload;
	}

	if((_flags & WSFlags::headerDeleteMemBit) != 0) {
		delete[] _header;
	}
}

uint8_t WebsocketFrameClass::encodeFrame(WSFrameType opcode, uint8_t* payload, size_t length, uint8_t mask, uint8_t fin,
										 uint8_t headerToPayload)
{
	if(length == 0 || length > 0xFFFF) {
		return false; //either zero length or too big for poor esp8266
	}

	uint8_t maskKey[4] = {0x00, 0x00, 0x00, 0x00};
	uint8_t* headerPtr = nullptr;
	uint8_t* dataPtr = nullptr;

	_payload = payload;
	_payloadLength = length;

	//calculate header size
	length < 126 ? _headerLength = 2 : _headerLength = 4;
	if(mask) {
		_headerLength += 4; // if present, mask is 4 bytes in header before payload
	}

	//try to allocate single buffer for both frame header and frame payload
	if(headerToPayload && (length < 1400) && (system_get_free_heap_size() > 6000)) {
		_payloadLength = length + _headerLength;
		dataPtr = new uint8_t[_payloadLength];
		if(dataPtr) {
			_flags |= WSFlags::payloadDeleteMemBit;
			os_memcpy((dataPtr + _headerLength), payload,
					  length); //copy original data to newly created buffer with _headerLength offset
			_payload = dataPtr;
		} else {
			headerToPayload =
				false; //memory allocation failed for single buffer, continue in light-memory mode with separate header and original payload
		}
	}

	// set Header Pointer
	if(headerToPayload) {
		_header = _payload; //Header is inside _payload buffer and occupy first _headerLength bytes
	} else {
		headerPtr = new uint8_t[_headerLength];
		if(headerPtr) {
			_header = headerPtr;
			_flags |= WSFlags::headerDeleteMemBit;
		} else {
			return false; //memory allocation failed
		}
	}

	os_memset(_header, 0, _headerLength); //set initial header state to be all zero

	// create header

	// byte 0
	if(fin) {
		*_header |= bit(7); // set Fin
	}
	*_header |= (uint8_t)opcode; // set opcode
	_header++;

	// byte 1
	if(mask) {
		*_header |= bit(7); // set mask
	}

	if(length < 126) {
		*_header |= length;
		_header++;
	} else if(length < 0xFFFF) {
		*_header |= 126;
		_header++;
		*_header = ((length >> 8) & 0xFF);
		_header++;
		*_header = (length & 0xFF);
		_header++;
	}

	if(mask && headerToPayload) {
		//we work on copy of original data so we can mask it without affecting original
		for(uint8_t x = 0; x < sizeof(maskKey); x++) {
			maskKey[x] = (char)os_random();
			*_header = maskKey[x];
			_header++;
		}

		uint8_t* dataMaskPtr = (_payload + _headerLength);

		for(size_t x = 0; x < length; x++) {
			dataMaskPtr[x] = (dataMaskPtr[x] ^ maskKey[x % 4]);
		}
	}

	if(headerToPayload) {
		_header =
			nullptr; //mark _header as nullptr to indicate external world that whole frame is referenced by _payload
		_headerLength = 0;
	} else {
		_header = headerPtr;
	}

	return true;
}

uint8_t WebsocketFrameClass::_getFrameSizes(uint8_t* buffer, size_t length)
{
	uint16_t payloadLength = buffer[1] & 0b01111111; // length of payload
	_mask = buffer[1] & 0b10000000;					 // extracting Mask bit

	//calculate header size
	payloadLength < 126 ? _headerLength = 2 : _headerLength = 4;

	if(_mask) {
		_headerLength += 4;
	}

	if(payloadLength == 126 && length < 4) {
		_frameType = WSFrameType::incomplete;
		return false;
	}

	if(payloadLength == 127) //unsupported frame size!
	{
		_frameType = WSFrameType::error;
		return false;
	}

	if(payloadLength == 126) {
		//next 2 bytes are length
		payloadLength = (buffer[2] << 8) | buffer[3];
	}

	_payloadLength = payloadLength;
	_nextReadOffset +=
		_headerLength + _payloadLength; // if given buffer is multiframe buffer store read offset for next frame

	if(length == _nextReadOffset) // check if given buffer is single frame buffer
	{
		_nextReadOffset = 0; // single websocket frame in buffer
	} else if(length < _nextReadOffset) {
		// Frame is incomplete
		_frameType = WSFrameType::incomplete;
		return false;
	}

	return true;
}

uint8_t WebsocketFrameClass::decodeFrame(uint8_t* buffer, size_t length)
{
	buffer += _nextReadOffset; // if called again for multiframe buffer, rewind buffer first to next frame
							   // no effect for single frame buffer as initial value of _nextReadOffset = 0

	WSFrameType op = (WSFrameType)(buffer[0] & 0b00001111); // Extracting Opcode
	uint8_t fin = buffer[0] & 0b10000000;					// Extracting Fin Bit (Single Frame)

	// At least there must be one byte that op and fin are vaild
	if(length > 0 && op == WSFrameType::continuation || op == WSFrameType::text ||
	   op == WSFrameType::binary) //Data frames
	{
		if(fin > 0) {
			uint8_t result = _getFrameSizes(buffer, length); // get _headerLength, _payloadLength and _mask

			if(!result) {
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

			if(_mask) {
				uint8_t mask[4];

				mask[0] = buffer[_headerLength - 4];
				mask[1] = buffer[_headerLength - 3];
				mask[2] = buffer[_headerLength - 2];
				mask[3] = buffer[_headerLength - 1];

				for(uint8_t i = 0; i < _payloadLength; i++) {
					_payload[i] ^= mask[i % 4];
				}
			}
		}
	}
	return true;
}

int WebsocketFrameClass::mask(const String& payload, uint32_t key, char* data)
{
	uint8_t pool[4] = {0};
	int pos = 0;
	pool[pos++] = (key >> 24) & 0xFF;
	pool[pos++] = (key >> 16) & 0xFF;
	pool[pos++] = (key >> 8) & 0xFF;
	pool[pos++] = (key >> 0) & 0xFF;

	int i;
	for(i = 0; i < payload.length(); i++) {
		data[i] = (payload[i] ^ pool[i % 4]);
	}

	return i;
}
