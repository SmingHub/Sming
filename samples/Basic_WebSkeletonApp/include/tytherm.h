#pragma once

#include "configuration.h"

extern unsigned long counter; // Kind of heartbeat counter

// Webserver
void startWebServer();

// STA disconnecter
const uint8_t StaConnectTimeout = 20; // 15 sec to connect in STA mode
void StaConnectOk();
void StaConnectFail();
