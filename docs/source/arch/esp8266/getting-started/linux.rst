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

Install Toolchain
-----------------

You can find pre-compiled toolchains in the `SmingTools <https://github.com/SmingHub/SmingTools/releases>`__ repository.

Install as follows::

   export ESP_HOME=/opt/esp-quick-toolchain
   sudo mkdir $(ESP_HOME)
   sudo chown $USER:$USER $(ESP_HOME)
   wget https://github.com/SmingHub/SmingTools/releases/download/1.0/x86_64-linux-gnu.xtensa-lx106-elf-e6a192b.201211.tar.gz
   tar -zxf x86_64-linux-gnu.xtensa-lx106-elf-e6a192b.201211.tar.gz -C $(ESP_HOME)

Espressif SDK
-------------

Sming uses the `Espressif 3.0.1 SDK <https://github.com/espressif/ESP8266_NONOS_SDK>`__
which is pulled in automatically during the build.

Previous versions are no longer officially supported.

Environment Variables
---------------------

From the command line::

   export ESP_HOME=/opt/esp-quick-toolchain
   export SMING_HOME=/opt/Sming/Sming

To set these permanently, add them to your home ``.profile`` file.

You can alternatively add them to ``/etc/environment`` for all users, like this::

   ESP_HOME="/opt/esp-quick-toolchain"
   SMING_HOME="/opt/Sming/Sming"


See also
`Setting up Sming and Netbeans <https://primalcortex.wordpress.com/2015/10/08/esp8266-setting-up-sming-and-netbeans/>`__
(external page).

Get Sming Core
--------------

Clone the Sming ``develop`` branch to your working directory::

   cd $SMING_HOME/../..
   git clone https://github.com/SmingHub/Sming.git
   cd Sming

.. warning::

   Do NOT use the ``--recursive`` option with ``git``.
   Our build mechanism will take care to get the third-party sources and patch them, if needed.

This will fetch the `develop` branch which is intended for developers,
and is the one where all new cool (unstable) features are landing.

If you want to use our stable branch::

   git checkout master
   git pull


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
