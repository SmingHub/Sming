/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Protobuf.h
 *
 ****/

#pragma once

#include "Protobuf/Stream.h"
#include "Protobuf/Callback.h"

namespace Protobuf
{
inline size_t getEncodeSize(const pb_msgdesc_t* fields, const void* src_struct)
{
	return DummyOutputStream().encode(fields, src_struct);
}

} // namespace Protobuf
