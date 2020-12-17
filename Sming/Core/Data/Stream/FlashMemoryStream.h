/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FlashMemoryStream.h
 *
 * @author: 23 Oct 2018 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include <FlashString/Stream.hpp>

/**
 * @brief Provides a read-only stream buffer on flash storage
 * @ingroup stream
 */
using FlashMemoryStream = FSTR::Stream;
