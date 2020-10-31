/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SeekOrigin.h
 *
 ****/

#pragma once

#include <WString.h>

/**
 * @brief Stream/file seek origins
 */
enum class SeekOrigin {
	Start = 0,   ///< SEEK_SET: Start of file
	Current = 1, ///< SEEK_CUR: Current position in file
	End = 2,	 ///< SEEK_END: End of file
};

String toString(SeekOrigin origin);
