#pragma once

#include <Print.h>
#include "Message.h"

namespace mDNS
{
void printQuestion(Print& p, Question& question);
void printAnswer(Print& p, Answer& answer);
void printMessage(Print& p, Message& message);

} // namespace mDNS
