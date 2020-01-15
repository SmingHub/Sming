/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Asn1Parser.h
 *
 * A very simple helper class consisting of code based on asn1_name() in axTLS.
 *
 ****/

#pragma once

#include <cstdint>

enum ASN1 {
	ASN1_BOOLEAN = 0x01,
	ASN1_INTEGER = 0x02,
	ASN1_BIT_STRING = 0x03,
	ASN1_OCTET_STRING = 0x04,
	ASN1_NULL = 0x05,
	ASN1_OID = 0x06,
	ASN1_PRINTABLE_STR2 = 0x0c,
	ASN1_PRINTABLE_STR = 0x13,
	ASN1_TELETEX_STR = 0x14,
	ASN1_IA5_STR = 0x16,
	ASN1_UTC_TIME = 0x17,
	ASN1_GENERALIZED_TIME = 0x18,
	ASN1_UNICODE_STR = 0x1e,
	ASN1_SEQUENCE = 0x30,
	ASN1_SET = 0x31,
	ASN1_IMPLICIT_TAG = 0x80,
	ASN1_CONTEXT_DNSNAME = 0x82,
	ASN1_EXPLICIT_TAG = 0xa0,
	ASN1_V3_DATA = 0xa3,
};

class Asn1Parser
{
public:
	Asn1Parser(const uint8_t* data, unsigned length) : buf(data), length(length)
	{
	}

	unsigned getOffset() const
	{
		return offset;
	}

	void setOffset(unsigned offset)
	{
		this->offset = offset;
	}

	unsigned getLength()
	{
		if((buf[offset] & 0x80) == 0) {
			return buf[offset++];
		}

		unsigned lengthBytes = buf[offset++] & 0x7f;
		if(lengthBytes > 4) {
			return 0;
		}

		uint32_t len = 0;
		for(unsigned i = 0; i < lengthBytes; i++) {
			len <<= 8;
			len += buf[offset++];
		}

		return len;
	}

	// Skip the ASN1.1 object type and its length. Get ready to read the object's data.
	unsigned getNextObject(uint8_t objType)
	{
		if(offset >= length || buf[offset] != objType) {
			return 0;
		}

		++offset;
		return getLength();
	}

	// Get the components of a distinguished name
	uint8_t getObjectId()
	{
		auto len = getNextObject(ASN1_OID);
		if(len == 0) {
			return 0;
		}

		uint8_t dnType = 0;

		/* expect a sequence of 2.5.4.[x] where x is a one of distinguished name
	       components we are interested in. */
		if(len == 3 && buf[offset] == 0x55 && buf[offset + 1] == 0x04) {
			dnType = buf[offset + 2];
		}

		// Skip it
		offset += len;
		return dnType;
	}

	// Extract a readable string
	String getString()
	{
		auto asn1_type = buf[offset];

		++offset;
		auto len = getLength();

		String s;
		switch(asn1_type) {
		case ASN1_UNICODE_STR:
			len /= 2;
			s.setLength(len);
			for(unsigned i = 0; i < len; ++i) {
				// Unicode MSB first
				s[i] = buf[offset + 1];
				offset += 2;
			}
			break;

		case ASN1_PRINTABLE_STR:
		case ASN1_PRINTABLE_STR2:
		case ASN1_TELETEX_STR:
		case ASN1_IA5_STR:
			s.setString(reinterpret_cast<const char*>(&buf[offset]), len);
			offset += len;
			break;
		}

		return s;
	}

private:
	const uint8_t* buf;
	unsigned length;
	unsigned offset = 0;
};
