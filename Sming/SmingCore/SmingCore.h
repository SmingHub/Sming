/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _NET_WIRING_
#define _NET_WIRING_

#define SMING_VERSION "3.2.0" // Major Minor Sub

#include "../Wiring/WiringFrameworkIncludes.h"

#include "Delegate.h"
#include "Boards.h"
#include "Clock.h"
#include "SystemClock.h"
#include "Digital.h"
#include "ESP8266EX.h"
#include "FileSystem.h"
#include "HardwareSerial.h"
#include "Interrupts.h"
#include "DriverPWM.h"
#include "HardwarePWM.h"
#include "Timer.h"
#include "Wire.h"
#include "SPISoft.h"
#include "SPI.h"


#include "Platform/System.h"
#include "Platform/WifiEvents.h"
#include "Platform/Station.h"
#include "Platform/AccessPoint.h"
#include "Platform/WDT.h"

#include "Network/DNSServer.h"
#include "Network/HttpClient.h"
#include "Network/MqttClient.h"
#include "Network/NtpClient.h"
#include "Network/HttpServer.h"
#include "Network/Http/HttpRequest.h"
#include "Network/Http/HttpResponse.h"
#include "Network/FTPServer.h"
#include "Network/NetUtils.h"
#include "Network/TcpClient.h"
#include "Network/TcpConnection.h"
#include "Network/UdpConnection.h"
#include "Network/rBootHttpUpdate.h"
#include "Network/URL.h"

#include "../Services/ArduinoJson/include/ArduinoJson.h"
#include "../Services/DateTime/DateTime.h"
#include "../Services/libemqtt/libemqtt.h"
#include "../Services/FATFS/ff.h"
#include "../Services/Yeelight/YeelightBulb.h"

#endif
