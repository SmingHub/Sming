# Sming
Sming is an asynchronous C/C++ framework with superb performance and multiple network features.
Sming is [open source](LICENSE) and is tailored towards embedded devices.
It supports multiple architectures as ESP8266 for example.

[![ESP8266 C++ development framework](https://github.com/SmingHub/Sming/wiki/images/small/combine.png)](https://github.com/SmingHub/Sming/wiki/examples)

[![Gitter (chat)](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/SmingHub/Sming?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
[![Backers](https://opencollective.com/Sming/backers/badge.svg)](#financial-contributions)
[![Sponsors](https://opencollective.com/Sming/sponsors/badge.svg)](#financial-contributions)
[![Download](https://img.shields.io/badge/download-~1.7M-orange.svg)](https://github.com/SmingHub/Sming/releases/latest)
[![Build](https://travis-ci.org/SmingHub/Sming.svg?branch=develop)](https://travis-ci.org/SmingHub/Sming)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/a450c9b4df08406dba81456261304ace)](https://app.codacy.com/app/slaff2/SmingOfficial?utm_source=github.com&utm_medium=referral&utm_content=SmingHub/Sming&utm_campaign=Badge_Grade_Dashboard)
[![Coverity Badge](https://img.shields.io/coverity/scan/12007.svg)](https://scan.coverity.com/projects/sminghub-sming)

If you like **Sming**, give it a star, or fork it and [contribute](#contribute)!

[![GitHub stars](https://img.shields.io/github/stars/SmingHub/Sming.svg?style=social&label=Star)](https://github.com/SmingHub/Sming/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/SmingHub/Sming.svg?style=social&label=Fork)](https://github.com/SmingHub/Sming/network)


Table of Contents
=================

* [Summary](#summary)
* [Compatibility](#compatibility)
   * [Architecture: ESP8266](#architecture-esp8266)
   * [Architecture: Host](#architecture-host)
* [Releases](#releases)
   * [Stable](#stable)
   * [Development](#development)
* [Getting Started](#getting-started)
* [Documentation](#documentation)
* [Examples](#examples)
   * [Basic Blink](#basic-blink)
   * [Simple GPIO Input/Output](#simple-gpio-inputoutput)
   * [Start Serial Communication](#start-serial-communication)
   * [Connect to WiFi](#connect-to-wifi)
   * [Read DHT22 sensor](#read-dht22-sensor)
   * [HTTP Client](#http-client)
   * [OTA Application Update Based on rBoot](#ota-application-update-based-on-rboot)
   * [HTTP Server](#http-server)
   * [Email Client](#email-client)
* [Live Debugging](#live-debugging)
* [Contribute](#contribute)
   * [Financial contributions](#financial-contributions)
      * [Backers and sponsors](#backers-and-sponsors)

Please note **develop** branch documentation is at [sming.readthedocs.io](https://sming.readthedocs.io/).

## Summary

* Superb performance and memory usage (Sming compiles to native firmware!).
* Fast and user friendly development.
* Simple yet powerful hardware API wrappers.
* Compatible with standard [Arduino Libraries](https://sming.readthedocs.io/en/latest/libraries.html) - use any popular hardware in few lines of code.
* Built-in file system: [spiffs](https://github.com/pellepl/spiffs).
* Built-in powerful wireless modules.
* Powerful asynchronous (async) network stack.
    * Async TCP and UDP stack based on [LWIP](http://savannah.nongnu.org/projects/lwip/).
    * With clients supporting: HTTP, MQTT, WebSockets and SMTP.
    * And servers for: DNS, FTP, HTTP(+ WebSockets), Telnet.
    * With SSL support for all network clients and servers based on [axTLS 2.1+](https://github.com/igrr/axtls-8266) with [Lwirax](https://github.com/attachix/lwirax/).
    * Out of the box support for OTA over HTTPS.
* ESP8266 specific features.
    * Integrated boot loader [rboot](https://sming.readthedocs.io/en/latest/_inc/Sming/Components/rboot/index.html) with support for 1MB ROMs, OTA firmware updating and ROM switching.
    * [Crash handlers](https://sming.readthedocs.io/en/latest/information/debugging.html) for analyzing/handling system restarts due to fatal errors or WDT resets.
    * [PWM support](https://sming.readthedocs.io/en/latest/_inc/Sming/Arch/Esp8266/Components/pwm_open/index.html) based on [Stefan Bruens PWM](https://github.com/StefanBruens/ESP8266_new_pwm.git).
    * Optional [custom heap allocation](https://sming.readthedocs.io/en/latest/_inc/Sming/Arch/Esp8266/Components/custom_heap/index.html) based on [Umm Malloc](https://github.com/rhempel/umm_malloc.git).
    * Based on Espressif NONOS SDK. Officially suppored NON SDK version is >= 3.0.1.
* Linux/Windows features
    * Sming has a [host emulator](https://sming.readthedocs.io/en/latest/arch/host/host-emulator.html) that allows libraries and sample applications to be compiled on a Linux/Windows host system and be tested before uploading them to an actual microcontroller.

## Compatibility

### Architecture: ESP8266

OS/SDK | Linux | Mac OS X | Windows | FreeBSD-current |
-------|-------|----------|---------|-----------------|
UDK (v1.5)    | n/a   | n/a      |   [![Build status](https://ci.appveyor.com/api/projects/status/5aj0oi0wyk4uij00/branch/develop?svg=true)](https://ci.appveyor.com/project/slaff/sming-sb483/branch/develop)      |     n/a         |
esp-open-sdk (v1.5.x **, v2.0.0 **, v3.0.x) | :sunny:  | :sunny: | n/a | :sunny: |

- OS = Operating System.
- SDK = Software Development Kit.
- n/a = The selected SDK is not available on that OS.
- ** = not supported officially. Samples and library should compile and run but we recommend using version >= 3.0.1.

### Architecture: Host

Linux and Windows OSes with gcc compilers are supported. Clang is NOT supported.


## Releases

### Stable

- [Sming V4.0.0](https://github.com/SmingHub/Sming/releases/tag/4.0.0) - great new features, performance and stability improvements.
- [Sming V3.8.1 LTS](https://github.com/SmingHub/Sming/releases/tag/3.8.1) - suitable for older applications that are not yet migrated to Sming v4.x.x.
Long Term Support (LTS) is available till 1st of April, 2020.


### Development

To follow the latest development you will need to clone our `develop` branch:

```
git clone https://github.com/SmingHub/Sming.git
```


## Getting Started

Sming supports multiple architectures. Choose the architecture of your choice to [install the needed development software](https://sming.readthedocs.io/en/latest/getting-started.html) and toolchain(s).

You can also try Sming without installing anything locally. We have an [interactive tutorial](https://www.katacoda.com/slaff/scenarios/sming-host-emulator) that can be run directly from your browser.

## Documentation
In addition to our [online documentation](https://sming.readthedocs.io/),
you can also generate a complete documentation locally by following these [instructions](https://sming.readthedocs.io/en/latest/_inc/docs/README.html).


## Examples
The examples are a great way to learn the API and brush up your C/C++ knowledge.
Once you have completed the installation of the development tools, you can get the latest source code.

```
git clone https://github.com/SmingHub/Sming.git
```

And check some of the examples.

- [Basic Blink](#basic-blink)
- [Simple GPIO input/output](#simple-gpio-inputoutput)
- [Start Serial communication](#start-serial-communication)
- [Connect to WiFi](#connect-to-wifi)
- [Read DHT22 sensor](#read-dht22-sensor)
- [HTTP Client](#http-client)
- [OTA Application Update Based on rBoot](#ota-application-update-based-on-rboot)
- [HTTP Server](#http-server)
- [Email Client](#email-client)

### Basic Blink
Blinking is something like the "Hello World" example for the embedded world. You can check it using the commands below:

```
cd Sming/samples
cd Basic_Blink
make # -- compiles the application
make flash # -- tries to upload the application to your ESP8266 device.
```

More information at **[Wiki Examples](https://github.com/SmingHub/Sming/wiki/examples)** page.

### Simple GPIO Input/Output
```c++
#define LED_PIN 2 // GPIO2
...
pinMode(LED_PIN, OUTPUT);
digitalWrite(LED_PIN, HIGH);
```

For a complete example take a look at the [Basic_Blink](samples/Basic_Blink/app/application.cpp) sample.

### Start Serial Communication
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

### HTTP Client
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

### OTA Application Update Based on rBoot
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

### HTTP Server
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

### Email Client
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
