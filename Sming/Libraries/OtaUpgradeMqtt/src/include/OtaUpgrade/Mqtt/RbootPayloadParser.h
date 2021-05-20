/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RbootPayloadParser.h
 *
 *  Created: 2021 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "StandardPayloadParser.h"

namespace OtaUpgrade
{
namespace Mqtt
{
using RbootPayloadParser = StandardPayloadParser SMING_DEPRECATED;

} // namespace Mqtt
} // namespace OtaUpgrade
