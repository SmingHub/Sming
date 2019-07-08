You will need Sming 1.1 or newer to support cross-platform tools. You
might want to use `sming-bootstrap <#>`__.

Install prerequisites
=====================

Ubuntu
------

.. code:: shell

   sudo apt-get install make unrar autoconf automake libtool libtool-bin gcc g++ gperf flex bison texinfo gawk ncurses-dev libexpat1-dev python sed python-serial python-dev srecord bc git help2man unzip bzip2

Fedora
------

.. code:: shell

   sudo dnf install make autoconf automake libtool gcc gcc-c++ gperf flex bison texinfo gawk ncurses-devel expat-devel python sed pyserial srecord bc git patch unzip help2man python-devel

Prepare directory
=================

*We recommend /opt/ but you can use anything you want*

::

   mkdir -p /opt/
   cd /opt/

Build esp-open-sdk
==================

(Or get precompiled esp-open-sdk 1.5.4
`(x86_64-bit) <https://www.dropbox.com/s/dx9tcqnx0yj61i3/esp-open-sdk-1.5.4-linux-x86_64.tar.gz?dl=1>`__
`(x86_32-bit) <https://www.dropbox.com/s/mzo7kp8nsnsfzc2/esp-open-sdk-1.5.4-linux-x86.tar.gz?dl=1>`__
`(Raspbian) <https://www.dropbox.com/s/b8omfjk9bzeo3dc/esp-open-sdk-1.5.4-linux-rpi.tar.gz?dl=1>`__
or 1.4.0
`(x86_64-bit) <https://www.dropbox.com/s/ge2km06rre1n6e0/esp-open-sdk-1.4.0-linux-x86_64.tar.gz?dl=1>`__
`(x86_32-bit) <https://www.dropbox.com/s/gblv9t13d4ybt42/esp-open-sdk-1.4.0-linux-x86.tar.gz?dl=1>`__
`(Raspbian) <https://www.dropbox.com/s/5yzdxa7hxzu41qz/esp-open-sdk-1.4.0-linux-rpi.tar.gz?dl=1>`__
)

::

   git clone --recursive https://github.com/pfalcon/esp-open-sdk.git
   cd esp-open-sdk

   sudo chown -R [username] ./ # be careful this command can do damage if used in the wrong directory, try without sudo first!
   make VENDOR_SDK=1.5.4 STANDALONE=y # It will take a while...

The parameter ``VENDOR_SDK=1.5.4`` specifies that you will be using SDK
version 1.5.4. Before changing this value to a newer version make sure
to check that the SDK version that you have chosen is
`supported <https://github.com/SmingHub/Sming#compatibility>`__.

Set ENV Variables
=================

.. code:: shell

   export ESP_HOME=/opt/esp-open-sdk
   export SMING_HOME=/opt/Sming/Sming

To Sming environment paths permanently: Just go to the Sming home
directory /opt/Sming/Sming and edit the file Makefile: because this
library has defaults for a nutter and programmers (like myself add
config options and never document them)

`more information
here <https://primalcortex.wordpress.com/2015/10/08/esp8266-setting-up-sming-and-netbeans/>`__

Get and Build Sming Core:
=========================

.. code:: shell

   cd $SMING_HOME/../..
   git clone https://github.com/SmingHub/Sming.git
   # Warning: Do NOT use the --recursive option for the command above. 
   #          Our build mechanism will take care to get the third-party sources and patch them, if needed.

   # You will get a copy of our `develop` branch which intended for developers 
   # and it is the one where all new cool (unstable) features are landing. 

   # If you want to use our stable branch type the command below
   cd Sming && git checkout origin/master

   cd Sming
   make

Install esptool.py:
===================

.. code:: shell

   sudo apt-get install python-serial unzip
   wget https://github.com/themadinventor/esptool/archive/master.zip
   unzip master.zip
   mv esptool-master $ESP_HOME/esptool
   rm master.zip

Install esptool2:
=================

.. code:: shell

   cd  $ESP_HOME
   git clone https://github.com/raburton/esptool2
   # For the command above: do NOT use the --recursive option as it will break the patching of the third-party libraries.
   cd esptool2
   make
   export PATH=$PATH:$ESP_HOME/esptool2

Build a ‘Basic Blink’ example:
==============================

.. code:: shell

   cd $SMING_HOME/../samples/Basic_Blink
   make

Build and flash a ‘Basic Blink’ example (using ttyUSB0):
========================================================

.. code:: shell

   cd $SMING_HOME/../samples/Basic_Blink
   make flash

Configuration
=============

You might want to configure your project before building. Edit
**Makefile-user.mk**
