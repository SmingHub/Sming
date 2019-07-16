Welcome to Sming's documentation
================================

.. highlight:: c++

**Let's do smart things!!!**

Sming is an Open Source framework for high efficiency WiFi SoC ESP8266
native development with C++ language.

.. image:: combine.png
   :target: https://github.com/SmingHub/Sming/wiki/examples


Summary
-------

-  Highly effective in performance and memory usage (this is native firmware!)
-  Simple and powerful hardware API wrappers
-  Fast & user friendly development
-  Work with GPIO in Arduino style
-  Compatible with standard Arduino libraries - use any popular hardware
   in few lines of code
-  Integrated boot loader `rBoot <https://github.com/raburton/rboot>`__
   with support for 1MB ROMs, OTA firmware updating and ROM switching
-  Built-in file system: `spiffs <https://github.com/pellepl/spiffs>`__
-  Built-in powerful wireless modules
-  Powerful asynchronous (async) network stack

   -  Async TCP and UDP stack based on `LWIP <http://savannah.nongnu.org/projects/lwip/>`__
   -  With clients supporting: HTTP, MQTT, WebSockets and SMTP
   -  And servers for: DNS, FTP, HTTP(+ WebSockets), Telnet
   -  With SSL support for all network clients and servers based on
      `axTLS 2.1+ <https://github.com/igrr/axtls-8266>`__ with
      `Lwirax <https://github.com/attachix/lwirax/>`__.
   -  Out of the box support for OTA over HTTPS.

-  Crash handlers for analyzing/handling system restarts due to fatal
   errors or WDT resets.
-  PWM support based on `Stefan Bruens
   PWM <https://github.com/StefanBruens/ESP8266_new_pwm.git>`__
-  Optional custom heap allocation based on `Umm
   Malloc <https://github.com/rhempel/umm_malloc.git>`__
-  Based on Espressif NONOS SDK. Tested with versions 1.5 and 2.0.
   Experimental support for SDK version >= 3.0.0.


Compilation and flashing
------------------------

See the getting started page for your respective operating system.

You can find more information about compilation and flashing process by
reading esp8266.com forum discussion thread. Official ESP8266
documentation can be found in the `Espressif
website <https://espressif.com/en/support/download/documents?keys=&field_type_tid%5B%5D=14>`__.

Examples
--------

Once you are ready with the “Getting started” guide you can get the
latest source code.

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
-  `Sending emails <#sending-emails>`__

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

     otaUpdater = new rBootHttpUpdate();

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

For more examples take a look at the
:sample:`HttpServer_ConfigNetwork`,
:sample:`HttpServer_Bootstrap`,
:sample:`HttpServer_WebSockets`
and :sample:`HttpServer_AJAX`
samples.

Sending emails
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
--------------

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

Documentation
-------------

We provide `generated
documentation <https://sminghub.github.io/Sming/api/>`__ online.

If you want you can also generate a complete documentation locally by
running the commands below. This requires ``doxygen`` to be installed on
your system.

.. code-block:: bash

   cd $SMING_HOME
   make docs

The newly generated documentation will be located under Sming/docs/api.

Contribute
----------

You can contribute to Sming by

- Providing Pull Requests with new features, bug fixes, new ideas, etc. Make sure to follow our :doc:`contribute/coding-style`. Read our :doc:`contribute/index` for details.
- Testing our latest source code and reporting issues.
- Supporting us financially to acquire hardware for testing and implementing or out of gratitude.

Financial contributions
~~~~~~~~~~~~~~~~~~~~~~~

We welcome financial contributions in full transparency on our `open
collective <https://opencollective.com/Sming>`__ page. They help us
improve the project and the community around it. If you would like to
support us you can `become a
backer <https://opencollective.com/Sming#backer>`__ or `a
sponsor <https://opencollective.com/Sming#sponsor>`__.

In addition to that anyone who is helping this project can file an
expense. If the expense makes sense for the development of the
community, it will be “merged” in the ledger of our open collective by
the core contributors and the person who filed the expense will be
reimbursed.

Backers and sponsors
^^^^^^^^^^^^^^^^^^^^

Thank you to all the people who have backed Sming or sponsored it.


.. toctree::
   :caption: Contents:
   :hidden:

   host-emulator
   features
   information/index
   experimental/index
   tools/index


Indices and tables
------------------

* :ref:`genindex`
