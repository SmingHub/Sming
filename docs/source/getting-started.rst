Getting Started
===============

.. highlight:: c++

Development System Installation
-------------------------------
Sming supports multiple architectures. Choose the architecture of your choice to install the needed development software and toolchain(s).


.. toctree::
   :titlesonly:
   :maxdepth: 1

   /arch/esp8266/getting-started/index
   /arch/host/host-emulator

You can also try Sming without installing anything locally.
We have an  `interactive tutorial <https://www.katacoda.com/slaff/scenarios/sming-host-emulator>`__ that can be run directly from your browser.

Documentation
-------------

In addition to our `online documentation <https://sming.readthedocs.io/>`__,
you can also generate a complete documentation locally by
following these :doc:`instructions </_inc/docs/README>`.


Examples
--------

The examples are a great way to learn the API and brush up your C/C++ knowledge.
Once you have completed the installation of the development tools, you can get the latest source code.

::

   git clone https://github.com/SmingHub/Sming.git

And check some of the examples.

-  `Basic Blink <#basic-blink>`__
-  `Simple GPIO input/output <#simple-gpio-inputoutput>`__
-  `Start Serial communication <#start-serial-communication>`__
-  `Connect to WiFi <#connect-to-wifi>`__
-  `Read DHT22 sensor <#read-dht22-sensor>`__
-  `HTTP client <#http-client>`__
-  `OTA application update based on rBoot <#ota-application-update-based-on-rboot>`__
-  `Embedded HTTP Web Server <#embedded-http-web-server>`__
-  `Email Client <#email-client>`__

Further documentation about the :doc:`samples` is available too.

Basic Blink
~~~~~~~~~~~

Blinking is something like the “Hello World” example for the embedded
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
   WifiStation.config("LOCAL-NETWORK", "123456789087"); // Put you SSID and Password here

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

OTA application update based on rBoot
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

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

For a complete example take a look at the :sample:`Basic_rBoot` sample.

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
~~~~~~~~~~~~~~

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

       if(!client.countPending()) {
           client.quit();
       }

       return 0;
   }

See the :sample:`SmtpClient` sample for details.

Live Debugging
~~~~~~~~~~~~~~

Applications based on Sming Framework that are flashed and running on an
ESP8266 device can be debugged using interactive debuggers. In order to
debug an application it has to be re-compiled with the ENABLE_GDB=1
directive. And then flashed on the device. As shown below:

.. code-block:: bash

   cd $SMING_HOME/../samples/LiveDebug
   make clean
   make ENABLE_GDB=1
   make flashapp # <-- this will update only the application firmware.

Once the debuggable application is flashed on the device the developers
have to run GDB. The easiest way to run the command-line GDB is to
execute the following command:

.. code-block:: bash

   make gdb

Developers using Eclipse CDT can have debugging sessions like the one
below:

.. :image:: /information/eclipse-debug-session.png

See :sample:`LiveDebug` sample for details.

