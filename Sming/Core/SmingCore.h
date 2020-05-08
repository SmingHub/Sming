/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SmingCore.h
 *
 ****/

#pragma once

#include "SmingVersion.h"

#include "gdb/gdb_hooks.h"
#include "WiringFrameworkIncludes.h"

#include "Delegate.h"
#include "Clock.h"
#include "SystemClock.h"
#include "Digital.h"
#include "FileSystem.h"
#include "HardwareSerial.h"
#include "Interrupts.h"
#include "HardwarePWM.h"
#include "Timer.h"
#include "Wire.h"
#include "SPISoft.h"
#include "SPI.h"

#include "Platform/RTC.h"
#include "Platform/System.h"
#include "Platform/WifiEvents.h"
#include "Platform/Station.h"
#include "Platform/AccessPoint.h"
#include "Platform/WDT.h"

#include "Network/DnsServer.h"
#include "Network/HttpClient.h"
#include "Network/MqttClient.h"
#include "Network/NtpClient.h"
#include "Network/HttpServer.h"
#include "Network/Http/HttpRequest.h"
#include "Network/Http/HttpResponse.h"
#include "Network/Http/Websocket/WebsocketConnection.h"
#include "Network/FtpServer.h"
#include "Network/NetUtils.h"
#include "Network/TcpClient.h"
#include "Network/TcpConnection.h"
#include "Network/UdpConnection.h"
#include "Network/Url.h"

#include "Data/Stream/FileStream.h"
#include "Data/Stream/TemplateFileStream.h"
#include "Data/Stream/FlashMemoryStream.h"

#include "DateTime.h"

#include "fatfs/ff.h"
