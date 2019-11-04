HttpServer Firmware Upload
==========================

Introduction
------------

The HTTP server coming with Sming is quite powerful but it is limited
from the available resources of the underlining hardware (your favorite
ESP8266 microcontroller).

This sample demonstrates how to enable file upload of the HTTP server.
On a normal computer the file uploads are usually using
temporary space on the hard disk or in memory to store the incoming data.

On an embedded device that is a luxury that we can hardly afford.
In this sample we demonstrate how to define which file upload fields
should be stored and what (file) streams are responsible for storing the data.
If a field is not specified then its content will be discarded.


Firmware upgrade via web interface
----------------------------------

The file upload feature is used to implement a simple firmware upgrade 
mechanism where the new firmware image is uploaded via the web browser.
As a security measure, firmware images must be cryptographically signed 
to prevent unauthorized parties from installing malicious firmware on 
your device. A public-key based signature algorithm is used. On first run, 
the build process will automatically generates a private 'signigning.key', 
which must be kept secret for obvious reasons. The corresponding public 
verification key is compiled into the firmware. A signed image is generated 
from the original ROM image and the signing.key using a small Python utility.


Usage instructions
------------------

1. 'make flash' to build and flash the example via USB cable. You need to do 
   this only once. Subsequent updates can be performed using the web interface.

2. 'make signedrom' to create the signed image in out/firmware/.../rom0.bin.signed

3. Point your browser to your ESP's IP address to open the firmware upgrade page.

4. Select the signed image created in step 2 and hit the "Update" button. 
   After a few seconds, you should see a confirmation that the upgrade was successful.
   The ESP now reboots into the new firmware. 
   
If the upgrade is not successful, check the serial console for error messages.


Limitations
-----------

For simplicity, this example assumes RBOOT_BIG_FLASH=1, i. e. the same ROM 
image can be used for both slots.
