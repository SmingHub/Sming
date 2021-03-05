/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Packet.h
 *
 ****/

#pragma once

#include <Core/Data/Packet.h>

namespace mDNS
{
/**
 * @brief Helper class for reading/writing packet content
 */
struct Packet : public NetworkPacket {
	using NetworkPacket::NetworkPacket;

	void writeName(const String& name)
	{
		size_t namepos{0};
		auto namelen = name.length();
		while(true) {
			int sep = name.indexOf('.', namepos);
			auto wordLength = (sep >= 0) ? (sep - namepos) : (namelen - namepos);
			if(wordLength == 0) {
				// Name ends in a '.' so add a pointer record; will fixup later
				write16(0xC000);
				break;
			}
			write8(wordLength);
			write(name.c_str() + namepos, wordLength);
			if(sep < 0) {
				write8(0); // End of name.
				break;
			}
			namepos = sep + 1;
		}
	}
};

} // namespace mDNS