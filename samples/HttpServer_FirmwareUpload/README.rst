HttpServer Firmware Upload
==========================

This example combines the :library:`MultipartParser` library for file uploads
and the :component:`OtaUpgrade` component, to create a browser based firmware 
upgrade solution akin to what's found in many consumer devices.
The example is kept as minimal as possible to serve as a starting point for your
own applications.


About HTTP server file upload
-----------------------------

The HTTP server coming with Sming is quite powerful but it is limited
by the available resources of the underlining hardware (your favorite
ESP8266 microcontroller).

This sample demonstrates how to use the :library:`MultipartParser` library
to enable file upload of the HTTP server. On a normal computer the file uploads
are usually using temporary space on the hard disk or in memory to store the
incoming data.

On an embedded device that is a luxury that we can hardly afford.
In this sample we demonstrate how to define which file upload fields
should be recognized and what (file) streams are responsible for processing and
storing the data.
If a field is not specified then its content will be discarded.


About OtaUpgrade
----------------

The OtaUpgrade component provides the :cpp:class:`OtaUpgradeStream` class which 
is hooked up to the web server to process a firmware upgrade file uploaded to
the device.
The component is also responsible for creating the upgrade files during the build 
process. A single upgrade file conveniently encapsulates all ROM images, thereby
relieving the user from having to know the slot that is updated and manually
selecting the corresponding ROM image in a Two-ROM configuration.
The file format also supports state-of-the-art security features like a digital
signature, encryption and downgrade protection. You are invited to play with  
them and observe their impact on code size. See also the :component:`OtaUpgrade` 
documentation for further advice on how to use the security features properly.


Usage instructions
------------------


1. Configure your flash memory layout:

   -  Set :envvar:`SPI_SIZE` to the flash memory size of your device.
   -  If necessary, modify :envvar:`RBOOT_ROM0_ADDR`, :envvar:`RBOOT_ROM1_ADDR`, 
      :envvar:`RBOOT_SPIFFS_0` and :envvar:`SPIFF_SIZE` to fit both ROM slots and
      the file system into the available flash memory. Make sure that the 
      flash areas do not overlap with each other or any the reserved regions.
      Refer to the :component:`rboot` documentation for further details.

2. Build the example by running::

      make

3. Connect your device via USB/Serial cable and run::

      make flashconfig
      
   to clear any remains of the previous flash layout configuration, followed by::

      make flash

   to install the example firmware. You need to do this only once. Subsequent 
   updates can be performed wirelessly using the web interface.

4. Point the browser to your ESP's IP address to open the firmware upgrade page.

5. Select the upgrade file, which has been automatically created alongside step 2
   from ``out/Esp8266/<build-type>/firmware/firmware.ota`` and hit the "Update" button.
   
   After a few seconds, you should see a confirmation the the upgrade was successful.
   The device now reboots into the upgraded firmware.
   
   If the upgrade is not successful, rebuild with debug output enabled and check the 
   serial console for error messages.
