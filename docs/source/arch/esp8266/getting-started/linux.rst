******************
Linux Installation
******************

.. highlight:: bash

Pre-requisites
==============

Ubuntu
------

::

   sudo apt-get install make unrar autoconf automake libtool libtool-bin gcc g++ gperf flex bison texinfo gawk ncurses-dev libexpat1-dev python sed python-serial python-dev srecord bc git help2man unzip bzip2

Fedora
------

::

   sudo dnf install make autoconf automake libtool gcc gcc-c++ gperf flex bison texinfo gawk ncurses-devel expat-devel python sed pyserial srecord bc git patch unzip help2man python-devel

Prepare directory
=================

*We recommend /opt/ but you can use anything you want*

::

   mkdir -p /opt/
   cd /opt/

Build esp-open-sdk
==================

This contains the toolchain required to build for the ESP8266.

::

   git clone --recursive https://github.com/pfalcon/esp-open-sdk.git
   cd esp-open-sdk

   # be careful this command can do damage if used
   # in the wrong directory, try without sudo first!
   sudo chown -R [username] ./

   # This will take a while...
   make STANDALONE=y

Here are the links for pre-compiled versions:

-  `x86 64-bit <https://www.dropbox.com/s/dx9tcqnx0yj61i3/esp-open-sdk-1.5.4-linux-x86_64.tar.gz?dl=1>`__
-  `x86 32-bit <https://www.dropbox.com/s/mzo7kp8nsnsfzc2/esp-open-sdk-1.5.4-linux-x86.tar.gz?dl=1>`__
-  `Raspbian <https://www.dropbox.com/s/b8omfjk9bzeo3dc/esp-open-sdk-1.5.4-linux-rpi.tar.gz?dl=1>`__

.. note::

   The above includes an older version of the Espressif SDK, but this is not used by Sming.
   
   Sming uses the Espressif 3.0.1 SDK which is pulled in automatically during the build.

   Previous versions are no longer officially supported.

Set ENV Variables
=================

::

   export ESP_HOME=/opt/esp-open-sdk
   export SMING_HOME=/opt/Sming/Sming

To set these permanently, add them to your home ``.profile`` file.

You can alternatively add them to ``/etc/environment`` for all users, like this:

::

   SMING_HOME="/opt/Sming/Sming"


See also
`Setting up Sming and Netbeans <https://primalcortex.wordpress.com/2015/10/08/esp8266-setting-up-sming-and-netbeans/>`__
(external page).

Get Sming Core
==============

::

   cd $SMING_HOME/../..
   git clone https://github.com/SmingHub/Sming.git
   # Warning: Do NOT use the --recursive option for the command above.
   #          Our build mechanism will take care to get the third-party
   #          sources and patch them, if needed.

   # You will get a copy of our `develop` branch which intended for developers
   # and it is the one where all new cool (unstable) features are landing.


   cd Sming

   # If you want to use our stable branch type the command below
   git checkout origin/master

Build a ‘Basic Blink’ example:
==============================

::

   cd $SMING_HOME/../samples/Basic_Blink
   make

Build and flash a ‘Basic Blink’ example (using ttyUSB0):
========================================================

::

   cd $SMING_HOME/../samples/Basic_Blink
   make flash

