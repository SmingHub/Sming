/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Base64OutputStream.h
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#pragma once

#include "../StreamTransformer.h"
#include "libb64/cencode.h"

/**
 * @brief      Base64 Stream
 * @ingroup    stream data
 *
 *  @{
*/

class Base64OutputStream : public StreamTransformer
{
public:
	/**
	 * @brief Stream that transforms bytes of data into base64 data stream
	 * @param stream - source stream
	 * @param resultSize The size of the intermediate buffer, created once per object and reused multiple times
	 */
	Base64OutputStream(IDataSourceStream* stream, size_t resultSize = 500);

	size_t transform(const uint8_t* source, size_t sourceLength, uint8_t* target, size_t targetLength) override;

	/**
	 * @brief A method that backs up the current state
	 */
	void saveState() override;

	/**
	 * @brief A method that restores the last backed up state
	 */
	void restoreState() override;

private:
	base64_encodestate state = {};
	base64_encodestate lastState = {};
};

/** @} */
