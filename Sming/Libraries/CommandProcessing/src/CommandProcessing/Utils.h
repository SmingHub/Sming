#pragma once

#include "Handler.h"
#include <HardwareSerial.h>

namespace CommandProcessing
{
void enable(Handler& commandHandler, HardwareSerial& serial);

} // namespace CommandProcessing
