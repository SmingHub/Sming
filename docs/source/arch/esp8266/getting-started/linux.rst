Linux Installation
==================

.. highlight:: bash

Pre-requisites
--------------

Ubuntu::

   sudo apt-get install make unrar autoconf automake libtool libtool-bin gcc g++ gperf \
      flex bison texinfo gawk ncurses-dev libexpat1-dev python sed python-serial \
      python-dev srecord bc git help2man unzip bzip2

Fedora::

   sudo dnf install make autoconf automake libtool gcc gcc-c++ gperf flex bison texinfo \
      gawk ncurses-devel expat-devel python sed pyserial srecord bc git patch \
      unzip help2man python-devel

Prepare directory
-----------------

We recommend ``/opt/`` but you can use anything you want::

   mkdir -p /opt/
   cd /opt/

Build/Install Toolchain
-----------------------

This builds the cross-compiler, linker, etc. required for the ESP8266::

   git clone --recursive https://github.com/pfalcon/esp-open-sdk.git
   cd esp-open-sdk

   # be careful this command can do damage if used
   # in the wrong directory, try without sudo first!
   sudo chown -R [username] ./

   # This will take a while...
   make STANDALONE=y

You can find pre-compiled toolchains in the `SmingTools <https://github.com/SmingHub/SmingTools/releases>`__ repository.

Espressif SDK
-------------

Sming uses the `Espressif 3.0.1 SDK <https://github.com/espressif/ESP8266_NONOS_SDK>`__
which is pulled in automatically during the build.

Previous versions are no longer officially supported.

Environment Variables
---------------------

From the command line::

   export ESP_HOME=/opt/esp-open-sdk
   export SMING_HOME=/opt/Sming/Sming

To set these permanently, add them to your home ``.profile`` file.

You can alternatively add them to ``/etc/environment`` for all users, like this::

   SMING_HOME="/opt/Sming/Sming"


See also
`Setting up Sming and Netbeans <https://primalcortex.wordpress.com/2015/10/08/esp8266-setting-up-sming-and-netbeans/>`__
(external page).

Get Sming Core
--------------

Clone the Sming ``develop`` branch to your working directory::

   cd $SMING_HOME/../..
   git clone https://github.com/SmingHub/Sming.git
   # Warning: Do NOT use the --recursive option for the command above.
   #          Our build mechanism will take care to get the third-party
   #          sources and patch them, if needed.

   # You will get a copy of our `develop` branch which intended for developers
   # and it is the one where all new cool (unstable) features are landing.


   cd Sming

If you want to use our stable branch::

   git checkout origin/master


Build a ‘Basic Blink’ example
-----------------------------

Change to the application directory and build::

   cd $SMING_HOME/../samples/Basic_Blink
   make

Flash to your device (using ttyUSB0)::

   cd $SMING_HOME/../samples/Basic_Blink
   make flash

Next steps
----------

Proceed to :doc:`config`.
