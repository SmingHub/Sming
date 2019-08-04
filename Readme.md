# Sming
Sming - Open Source framework for high efficiency WiFi SoC ESP8266 native development with C++ language.

[![ESP8266 C++ development framework](https://github.com/SmingHub/Sming/wiki/images/small/combine.png)](https://github.com/SmingHub/Sming/wiki/examples)

[![Gitter (chat)](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/SmingHub/Sming?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
[![Backers](https://opencollective.com/Sming/backers/badge.svg)](#financial-contributions)
[![Sponsors](https://opencollective.com/Sming/sponsors/badge.svg)](#financial-contributions)
[![Download](https://img.shields.io/badge/download-~1.7M-orange.svg)](https://github.com/SmingHub/Sming/releases/latest)
[![Build](https://travis-ci.org/SmingHub/Sming.svg?branch=develop)](https://travis-ci.org/SmingHub/Sming)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/a450c9b4df08406dba81456261304ace)](https://app.codacy.com/app/slaff2/SmingOfficial?utm_source=github.com&utm_medium=referral&utm_content=SmingHub/Sming&utm_campaign=Badge_Grade_Dashboard)

If you like **Sming**, give it a star, or fork it and [contribute](#contribute)!

[![GitHub stars](https://img.shields.io/github/stars/SmingHub/Sming.svg?style=social&label=Star)](https://github.com/SmingHub/Sming/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/SmingHub/Sming.svg?style=social&label=Fork)](https://github.com/SmingHub/Sming/network)

## Summary
* Highly effective in performance and memory usage (this is native firmware!)
* Simple and powerful hardware API wrappers
* Fast & user friendly development
* Work with GPIO in Arduino style
* Compatible with standard Arduino libraries - use any popular hardware in few lines of code
* Integrated boot loader [rBoot](https://github.com/raburton/rboot) with support for 1MB ROMs, OTA firmware updating and ROM switching
* Built-in file system: [spiffs](https://github.com/pellepl/spiffs)
* Built-in powerful wireless modules
* Powerful asynchronous (async) network stack
    * Async TCP and UDP stack based on [LWIP](http://savannah.nongnu.org/projects/lwip/)
    * With clients supporting: HTTP, MQTT, WebSockets and SMTP
    * And servers for: DNS, FTP, HTTP(+ WebSockets), Telnet
    * With SSL support for all network clients and servers based on [axTLS 2.1+](https://github.com/igrr/axtls-8266) with [Lwirax](https://github.com/attachix/lwirax/).
    * Out of the box support for OTA over HTTPS.
* Crash handlers for analyzing/handling system restarts due to fatal errors or WDT resets.
* PWM support based on [Stefan Bruens PWM](https://github.com/StefanBruens/ESP8266_new_pwm.git)
* Optional custom heap allocation based on [Umm Malloc](https://github.com/rhempel/umm_malloc.git)
* Based on Espressif NONOS SDK. Tested with versions 1.5 and 2.0. Experimental support for SDK version >= 3.0.0.

## Compatibility

OS/SDK | Linux | Mac OS X | Windows | FreeBSD-current |
-------|-------|----------|---------|-----------------|
UDK (v1.5)    | n/a   | n/a      |   [![Build status](https://ci.appveyor.com/api/projects/status/5aj0oi0wyk4uij00/branch/develop?svg=true)](https://ci.appveyor.com/project/slaff/sming-sb483/branch/develop)      |     n/a         |
esp-open-sdk (v1.5.x, v2.0.0, v3.0 **) | :sunny:  | :sunny: | n/a | :sunny: |

- OS = Operating System.
- SDK = Software Development Kit.
- n/a = The selected SDK is not available on that OS.
- ** = experimental support

## Latest Stable Release
- [Sming V3.8.0](https://github.com/SmingHub/Sming/releases/tag/3.8.0)


## Getting started
- [Windows](https://github.com/SmingHub/Sming/wiki/Windows-Quickstart)
- [Linux](https://github.com/SmingHub/Sming/wiki/Linux-Quickstart)
- [MacOS](https://github.com/SmingHub/Sming/wiki/MacOS-Quickstart)
- [Docker](https://github.com/SmingHub/Sming/wiki/Docker-Quickstart)

## Optional features

<details><summary>There are multiple custom features that can be enabled by default. For example: SSL support, custom LWIP, open PWM, custom heap allocation, more verbose debugging, etc. Click here to see the details</summary><p>

- **Custom LWIP**: (default: ON) By default we are using custom compiled LWIP stack instead of the binary one provided from Espressif. This increases the free memory and decreases the space on the flash. All espconn_* functions are turned off by default. If your application requires the use of some of the espconn_* functions then add the ENABLE_ESPCONN=1 directive. See `Makefile-user.mk` from the [Basic_SmartConfig](https://github.com/SmingHub/Sming/blob/develop/samples/Basic_SmartConfig/Makefile-user.mk#L41) application for examples. If you would like to use the binary LWIP then you should turn off the custom LWIP compilation by providing `ENABLE_CUSTOM_LWIP=0`.
- **LWIP 2**: (default: OFF) LWIP 2 stands for LightWeight IP stack, version 2. In order to enable that feature you should (re)compile the Sming library AND your application using the following directive ENABLE_CUSTOM_LWIP=2. LWIP v2 does not have support for espconn_* functions.
This feature is still **experimental** which means that we do not recommend it in production.
- **SSL support**: (default: OFF) The SSL support is not built-in by default to conserve resources. If you want to enable it then take a look at the [Readme](https://github.com/SmingHub/Sming/blob/develop/samples/Basic_Ssl/README.md) in the Basic_Ssl samples.
- **Custom PWM**: (default: ON) If you don't want to use the [open PWM implementation](https://github.com/StefanBruens/ESP8266_new_pwm) then compile your application with `ENABLE_CUSTOM_PWM=0`. There is no need to recompile the Sming library.
- WPS: (default: OFF) The WPS support (Wi-Fi Protected Setup) is not activated by default to preserve resources. To enable WPS, use the switch ENABLE_WPS=1 for compiling Sming.
- **Custom serial baud rate**: (default: OFF) The default serial baud rate is 115200. If you want to change it to a different baud rate you can recompile Sming and your application changing the `COM_SPEED_SERIAL` directive. For example `COM_SPEED_SERIAL=921600`.
- **Custom heap allocation**: (default: OFF) If your application is experiencing heap fragmentation then you can try the [umm_malloc](https://github.com/rhempel/umm_malloc) heap allocation. To enable it compile Sming with `ENABLE_CUSTOM_HEAP=1`. In order to use it in your sample/application make sure to compile the sample with `ENABLE_CUSTOM_HEAP=1`. **Do not enable custom heap allocation and -mforce-l32 compiler flag at the same time**.
- **Debug information log level and format**: There are four debug levels: debug=3, info=2, warn=1, error=0. Using `DEBUG_VERBOSE_LEVEL` you can set the desired level (0-3). For example `DEBUG_VERBOSE_LEVEL=2` will show only info messages and above. Another make directive is `DEBUG_PRINT_FILENAME_AND_LINE=1` which enables printing the filename and line number of every debug line. This will require extra space on flash. Note: you can compile the Sming library with a set of debug directives and your project with another settings, this way you can control debugging separately for Sming and your application code.
- **Debug information for custom LWIP**: If you use custom LWIP (see above) some debug information will be printed for critical errors and situations. You can enable all debug information printing using `ENABLE_LWIPDEBUG=1`. To increase debugging for certain areas you can modify debug options in `third-party/esp-open-lwip/include/lwipopts.h`.
- **Interactive debugging on the device**: (default: OFF) In order to be able to debug live directly on the ESP8266 microcontroller you should re-compile the Sming library and your application with `ENABLE_GDB=1` directive. See [LiveDebug](https://github.com/SmingHub/Sming/tree/develop/samples/LiveDebug) sample for more details.
- **CommandExecutor feature**: (default: ON) This feature enables execution of certain commands by registering token handlers for text received via serial, websocket or telnet connection. If this feature is not used additional RAM/Flash can be obtained by setting `ENABLE_CMD_EXECUTOR=0`. This will save ~1KB RAM and ~3KB of flash memory.
- **SDK 3.0+**: (default: OFF) In order to use SDK 3.0.0 or newer you should set one environment variable before (re)compiling Sming AND applications based on it.  The variable is SDK_BASE and it should point to `$SMING_HOME/third-party/ESP8266_NONOS_SDK`.

For Windows you need to do:
```
set SDK_BASE %SMING_HOME%//third-party/ESP8266_NONOS_SDK
```

For Linux(bash) you need to do:
```
export SDK_BASE="$SMING_HOME/third-party/ESP8266_NONOS_SDK"
```

Read the comments from [this URL](https://github.com/SmingHub/Sming/pull/1264) for known issues.
This feature is still **experimental** which means that we still do not recommend it in production.
</p></details>

## Compilation and flashing
See the getting started page for your respective operating system.

You can find more information about compilation and flashing process by reading esp8266.com forum discussion thread.
Official ESP8266 documentation can be found in the [Espressif website](https://espressif.com/en/support/download/documents?keys=&field_type_tid%5B%5D=14).

## Examples
Once you are ready with the "Getting started" guide you can get the latest source code.

```
git clone https://github.com/SmingHub/Sming.git
```

And check some of the examples.

- [Basic Blink](#basic-blink)
- [Simple GPIO input/output](#simple-gpio-inputoutput)
- [Start Serial communication](#start-serial-communication)
- [Connect to WiFi](#connect-to-wifi)
- [Read DHT22 sensor](#read-dht22-sensor)
- [HTTP client](#http-client)
- [OTA application update based on rBoot](#ota-application-update-based-on-rboot)
- [Embedded HTTP Web Server](#embedded-http-web-server)
- [Sending emails](#sending-emails)

### Basic Blink
Blinking is something like the "Hello World" example for the embedded world. You can check it using the commands below:

```
cd Sming/samples
cd Basic_Blink
make # -- compiles the application
make flash # -- tries to upload the application to your ESP8266 device.
```

More information at **[Wiki Examples](https://github.com/SmingHub/Sming/wiki/examples)** page.

### Simple GPIO input/output
```c++
#define LED_PIN 2 // GPIO2
...
pinMode(LED_PIN, OUTPUT);
digitalWrite(LED_PIN, HIGH);
```

For a complete example take a look at the [Basic_Blink](samples/Basic_Blink/app/application.cpp) sample.

### Start Serial communication
```c++
Serial.begin(9600);
Serial.println("Hello Sming! Let's do smart things.");
```

### Connect to WiFi
```c++
WifiStation.enable(true);
WifiStation.config("LOCAL-NETWORK", "123456789087"); // Put you SSID and Password here
```

### Read DHT22 sensor
```c++
#include <Libraries/DHTesp/DHTesp.h> // This is just a popular Arduino library!

#define DHT_PIN 0 // GPIO0
DHTesp dht;

void init()
{
  dht.setup(DHT_PIN, DHTesp::DHT22);

  float h = dht.getHumidity();
  float t = dht.getTemperature();
}
```

Take a look at the code of the [Humidity_DHT22](samples/Humidity_DHT22/app/application.cpp) sample.

### HTTP client
```c++
HttpClient thingSpeak;
...
thingSpeak.downloadString("http://api.thingspeak.com/update?key=XXXXXXX&field1=" + String(sensorValue), onDataSent);

void onDataSent(HttpClient& client, bool successful)
{
  if (successful) {
    Serial.println("Successful!");
  }
  else {
    Serial.println("Failed");
  }
}
```

For more examples take a look at the [HttpClient](samples/HttpClient/app/application.cpp), [HttpClient_Instapush](samples/HttpClient_Instapush/app/application.cpp) and [HttpClient_ThingSpeak](samples/HttpClient_ThingSpeak/app/application.cpp) samples.

### OTA application update based on rBoot
```c++
void OtaUpdate()
{
  uint8 slot;
  rboot_config bootconf;

  Serial.println("Updating...");

  // need a clean object, otherwise if run before and failed will not run again
  if (otaUpdater) {
    delete otaUpdater;
  }

  otaUpdater = new RbootHttpUpdater();

  // select rom slot to flash
  bootconf = rboot_get_config();
  slot = bootconf.current_rom;
  if (slot == 0) {
    slot = 1;
  }
  else {
    slot = 0;
  }

  // flash rom to position indicated in the rBoot config rom table
  otaUpdater->addItem(bootconf.roms[slot], ROM_0_URL);

  // and/or set a callback (called on failure or success without switching requested)
  otaUpdater->setCallback(OtaUpdate_CallBack);

  // start update
  otaUpdater->start();
}
```

For a complete example take a look at the [Basic_rBoot](samples/Basic_rBoot/app/application.cpp) sample.

### Embedded HTTP Web Server
```c++
server.listen(80);
server.paths.set("/", onIndex);
server.paths.set("/hello", onHello);
server.paths.setDefault(onFile);

Serial.println("=== WEB SERVER STARTED ===");
Serial.println(WifiStation.getIP());

...

void onIndex(HttpRequest &request, HttpResponse &response)
{
  TemplateFileStream *tmpl = new TemplateFileStream("index.html");
  auto &vars = tmpl->variables();
  vars["counter"] = String(counter);
  vars["IP"] = WifiStation.getIP().toString();
  vars["MAC"] = WifiStation.getMAC();
  response.sendTemplate(tmpl);
}

void onFile(HttpRequest &request, HttpResponse &response)
{
  String file = request.getPath();
  if (file[0] == '/')
    file = file.substring(1);

  response.setCache(86400, true);
  response.sendFile(file);
}
```

For more examples take a look at the [HttpServer_ConfigNetwork](samples/HttpServer_ConfigNetwork/app/application.cpp), [HttpServer_Bootstrap](samples/HttpServer_Bootstrap/app/application.cpp), [HttpServer_WebSockets](samples/HttpServer_WebSockets/app/application.cpp) and [HttpServer_AJAX](samples/HttpServer_AJAX/app/application.cpp) samples.

### Sending emails
```c++
SmtpClient emailClient;

emailClient.connect(Url("smtp://user:password@domain.com"));

MailMessage* mail = new MailMessage();
mail->from = "developers@sming";
mail->to = "iot-developers@world";
mail->subject = "Greetings from Sming";
mail->setBody("Hello");

FileStream* file= new FileStream("image.png");
mail->addAttachment(file);

emailClient.onMessageSent(onMailSent);
emailClient.send(mail);

...

int onMailSent(SmtpClient& client, int code, char* status)
{
    MailMessage* mail = client.getCurrentMessage();

    ...

    if(!client.countPending()) {
        client.quit();
    }

    return 0;
}

```

See the [SmtpClient sample](samples/SmtpClient/app/application.cpp) for details.

## Live Debugging
Applications based on Sming Framework that are flashed and running on an ESP8266 device can be debugged using interactive debuggers.
In order to debug an application it has to be re-compiled with the ENABLE_GDB=1 directive. And then flashed on the device. As shown below:

```
cd $SMING_HOME/../samples/LiveDebug
make clean
make ENABLE_GDB=1
make flashapp # <-- this will update only the application firmware.
```

Once the debuggable application is flashed on the device the developers have to run GDB. The easiest way to run the command-line GDB is to execute the following command:
```
make gdb
```

Developers using Eclipse CDT can have debugging sessions like the one below:
![Debugging Session in Eclipse CDT](https://raw.githubusercontent.com/SmingHub/Sming/gh-pages/images/eclipse-debug-session.png)

See [LiveDebug sample](samples/LiveDebug/) for details.

## Documentation
We provide [generated documentation](https://sminghub.github.io/Sming/api/) online.

If you want you can also generate a complete documentation locally by running the commands below. This requires `doxygen` to be installed on your system.

```
cd $SMING_HOME
make docs
```

The newly generated documentation will be located under Sming/docs/api.


## Contribute

You can contribute to Sming by
- Providing Pull Requests with new features, bug fixes, new ideas, etc.
Make sure to follow our [Coding-Style-Rules](https://github.com/SmingHub/Sming/wiki/Coding-Style-Rules). Read our [Contributing guide](https://github.com/SmingHub/Sming/blob/develop/CONTRIBUTING.md) for details.
- Testing our latest source code and reporting issues.
- Supporting us financially to acquire hardware for testing and implementing or out of gratitude

### Financial contributions

We welcome financial contributions in full transparency on our [open collective](https://opencollective.com/Sming) page.
They help us improve the project and the community around it. If you would like to support us you can [become a backer](https://opencollective.com/Sming#backer) or [a sponsor](https://opencollective.com/Sming#sponsor).

In addition to that anyone who is helping this project can file an expense. If the expense makes sense for the development of the community, it will be "merged" in the ledger of our open collective by the core contributors and the person who filed the expense will be reimbursed.


#### Backers and sponsors

Thank you to all the people who have backed Sming
<a href="https://opencollective.com/Sming#backers" target="_blank"><img src="https://opencollective.com/Sming/backers.svg?width=890"></a>

or sponsored it.

<a href="https://opencollective.com/Sming/sponsor/0/website" target="_blank"><img src="https://opencollective.com/Sming/sponsor/0/avatar.svg"></a>

