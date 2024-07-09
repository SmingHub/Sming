# Sming

Sming is an asynchronous embedded C/C++ framework with superb performance and multiple network features. 
Sming is [open source](LICENSE), modular and supports [multiple architectures](https://sming.readthedocs.io/en/latest/features.html) including ESP8266, ESP32 and RP2040.

[![Examples](https://github.com/SmingHub/Sming/wiki/images/small/combine.png)](https://github.com/SmingHub/Sming/wiki/examples)

[![Gitter (chat)](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/SmingHub/Sming?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
[![Backers](https://opencollective.com/Sming/backers/badge.svg)](#financial-contributions)
[![Sponsors](https://opencollective.com/Sming/sponsors/badge.svg)](#financial-contributions)
[![Download](https://img.shields.io/badge/download-~1.7M-orange.svg)](https://github.com/SmingHub/Sming/releases/latest)
[![Build](https://github.com/SmingHub/Sming/actions/workflows/ci.yml/badge.svg)](https://github.com/SmingHub/Sming/actions/workflows/ci.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/23ff16f8d550440787125b0d25ba7ada)](https://www.codacy.com/gh/SmingHub/Sming/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=SmingHub/Sming&amp;utm_campaign=Badge_Grade)
[![Coverity Badge](https://img.shields.io/coverity/scan/12007.svg)](https://scan.coverity.com/projects/sminghub-sming)

If you like **Sming**, give it a star, or fork it and [contribute](#contribute)!

[![GitHub stars](https://img.shields.io/github/stars/SmingHub/Sming.svg?style=social&label=Star)](https://github.com/SmingHub/Sming/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/SmingHub/Sming.svg?style=social&label=Fork)](https://github.com/SmingHub/Sming/network)

## Getting Started

Sming supports multiple architectures and has a [plethora of features](https://sming.readthedocs.io/en/latest/index.html#summary).
Choose the architecture of your choice to [install the needed development software](https://sming.readthedocs.io/en/latest/getting-started).

You can also try Sming without installing anything locally. We have an [interactive tutorial](https://killercoda.com/slaff/scenario/sming-host-emulator) that can be run directly from your browser.

## Documentation

The purpose of Sming is to simplify the creation of embedded applications. The documentation will help you get started in no time.

- [**Documentation for version 5.1.0**](https://sming.readthedocs.io/en/stable) - current stable version.
- [Documentation for latest](https://sming.readthedocs.io/en/latest) - development version.

## Releases

### Stable

- [Sming V5.1.0](https://github.com/SmingHub/Sming/releases/tag/5.1.0) - great new features, performance and stability improvements.

### Development

To follow the latest development you will need to clone our `develop` branch:

```
git clone https://github.com/SmingHub/Sming.git
```


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
- [OTA Application Update](#ota-application-update)
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

More information at **[Sample Projects](https://sming.readthedocs.io/en/latest/samples.html)** page.

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
WifiStation.config("LOCAL-NETWORK", "123456789087"); // Put your SSID and password here
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

### OTA Application Update
```c++
void doUpgrade()
{
  // need a clean object, otherwise if run before and failed will not run again
  if(otaUpdater) {
      delete otaUpdater;
  }
  otaUpdater = new Ota::Network::HttpUpgrader();

  // select rom partition to flash
  auto part = ota.getNextBootPartition();

  // The content located on ROM_0_URL will be stored to the new partition
  otaUpdater->addItem(ROM_0_URL, part);

  // and/or set a callback (called on failure or success without switching requested)
  otaUpdater->setCallback(upgradeCallback);

  // start update
  otaUpdater->start();
}
```

For a complete example take a look at the [Basic_Ota](samples/Basic_Ota/app/application.cpp) sample.

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
- Providing Pull Requests with new features, bug fixes, new ideas, etc. See [Contributing](https://smingdev.readthedocs.io/en/latest/contribute/index.html) for details.
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
