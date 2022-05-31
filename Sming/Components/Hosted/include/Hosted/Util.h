#pragma once

#include <WString.h>

namespace Hosted
{
/**
 * Convert C type to format character
 * See: https://docs.python.org/3.5/library/struct.html#format-strings
 */
char convertType(const String& type);

/**
 * Converts a name as given from __PRETTY_FUNCTION__ to internal format.
 * Examples:
 * 			void a::sub(int) -> a::sub(: i)
 * 			void TwoWire::pins(uint8_t,uint8_t) -> TwoWire::pins(: B B)
 * 			uint8_t digitalRead(uint16_t) -> digitalRead(B: H)
 * @param name source name
 * @retval converted name
 */
String convertFQN(const String& name);

} // namespace Hosted
