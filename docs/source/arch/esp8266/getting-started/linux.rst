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

(Alternatively, get precompiled esp-open-sdk 1.5.4:
`x86_64-bit <https://www.dropbox.com/s/dx9tcqnx0yj61i3/esp-open-sdk-1.5.4-linux-x86_64.tar.gz?dl=1>`__,
`x86_32-bit <https://www.dropbox.com/s/mzo7kp8nsnsfzc2/esp-open-sdk-1.5.4-linux-x86.tar.gz?dl=1>`__,
`Raspbian <https://www.dropbox.com/s/b8omfjk9bzeo3dc/esp-open-sdk-1.5.4-linux-rpi.tar.gz?dl=1>`__
or 1.4.0:
`x86_64-bit <https://www.dropbox.com/s/ge2km06rre1n6e0/esp-open-sdk-1.4.0-linux-x86_64.tar.gz?dl=1>`__,
`x86_32-bit <https://www.dropbox.com/s/gblv9t13d4ybt42/esp-open-sdk-1.4.0-linux-x86.tar.gz?dl=1>`__,
`Raspbian <https://www.dropbox.com/s/5yzdxa7hxzu41qz/esp-open-sdk-1.4.0-linux-rpi.tar.gz?dl=1>`__.)

::

   git clone --recursive https://github.com/pfalcon/esp-open-sdk.git
   cd esp-open-sdk

   # be careful this command can do damage if used
   # in the wrong directory, try without sudo first!
   sudo chown -R [username] ./

   # This will take a while...
   make VENDOR_SDK=1.5.4 STANDALONE=y

The parameter ``VENDOR_SDK=1.5.4`` specifies that you will be using SDK
version 1.5.4. Before changing this value to a newer version make sure
to check that the SDK version that you have chosen is
`supported <https://github.com/SmingHub/Sming#compatibility>`__.

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

