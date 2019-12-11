#pragma once
#include <SmingCore.h>

void mbLoop(void);
void preTransmission(void);
void postTransmission(void);
void mbLogReceive(const uint8_t*, size_t, uint8_t);
void mbLogTransmit(const uint8_t*, size_t);
