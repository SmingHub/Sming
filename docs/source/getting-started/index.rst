Getting Started
===============

.. highlight:: c++

Development System Installation
-------------------------------

Choose your preferred development environment for how to install the needed development software and toolchain(s):

.. toctree::
   :titlesonly:
   :maxdepth: 1

   linux/index
   macos/index
   windows/index
   docker/index

You can also try Sming without installing anything locally.
We have an  `interactive tutorial <https://killercoda.com/slaff/scenario/sming-host-emulator>`__ that can be run directly from your browser.


.. toctree::
   :titlesonly:
   :maxdepth: 1

   config


Documentation
-------------

In addition to our `online documentation <https://sming.readthedocs.io/>`__,
you can also generate a complete documentation locally by
following these :doc:`instructions </_inc/docs/README>`.


Examples
--------

The examples are a great way to learn the API and brush up your C++ knowledge.

-  `Basic Blink <#basic-blink>`__
-  `Simple GPIO input/output <#simple-gpio-inputoutput>`__
-  `Start Serial communication <#start-serial-communication>`__
-  `Connect to WiFi <#connect-to-wifi>`__
-  `Read DHT22 sensor <#read-dht22-sensor>`__
-  `HTTP client <#http-client>`__
-  `OTA application update based on rBoot <#ota-application-update>`__
-  `Embedded HTTP Web Server <#embedded-http-web-server>`__
-  `Email Client <#email-client>`__

Further documentation about the :doc:`/samples` is available too.

Basic Blink
~~~~~~~~~~~

Blinking is something like the "Hello World" example for the embedded
world. You can check it using the commands below:

.. code-block:: bash

   cd Sming/samples
   cd Basic_Blink
   make # -- compiles the application
   make flash # -- tries to upload the application to your ESP8266 device.

See :sample:`Basic_Blink` for more information.

Simple GPIO input/output
~~~~~~~~~~~~~~~~~~~~~~~~

::

   #define LED_PIN 2 // GPIO2
   ...
   pinMode(LED_PIN, OUTPUT);
   digitalWrite(LED_PIN, HIGH);

For a complete example take a look at the :sample:`Basic_Blink` sample.

Start Serial communication
~~~~~~~~~~~~~~~~~~~~~~~~~~

::

   Serial.begin(9600);
   Serial.println("Hello Sming! Let's do smart things.");

Connect to WiFi
~~~~~~~~~~~~~~~

::

   WifiStation.enable(true);
   WifiStation.config("LOCAL-NETWORK", "123456789087"); // Put your SSID and password here

Read DHT22 sensor
~~~~~~~~~~~~~~~~~

::

   #include <Libraries/DHTesp/DHTesp.h> // This is just a popular Arduino library!

   #define DHT_PIN 0 // GPIO0
   DHTesp dht;

   void init()
   {
     dht.setup(DHT_PIN, DHTesp::DHT22);

     float h = dht.getHumidity();
     float t = dht.getTemperature();
   }

Take a look at the code of the :sample:`Humidity_DHT22` sample.

HTTP client
~~~~~~~~~~~

::

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

For more examples take a look at the
:sample:`HttpClient`,
:sample:`HttpClient_Instapush`
and :sample:`HttpClient_ThingSpeak` samples.

OTA application update
~~~~~~~~~~~~~~~~~~~~~~

::

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

For a complete example take a look at the :sample:`Basic_Ota` sample.

Embedded HTTP Web Server
~~~~~~~~~~~~~~~~~~~~~~~~

::

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
     vars["MAC"] = WifiStation.getMacAddress().toString();
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

For more examples take a look at the
:sample:`HttpServer_ConfigNetwork`,
:sample:`HttpServer_Bootstrap`,
:sample:`HttpServer_WebSockets`
and :sample:`HttpServer_AJAX`
samples.

Email client
~~~~~~~~~~~~

::

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

       if(client.countPending() == 0) {
           client.quit();
       }

       return 0;
   }

See the :sample:`SmtpClient` sample for details.
