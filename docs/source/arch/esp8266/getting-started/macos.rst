Mac-OS Installation
===================

.. highlight:: bash

Pre-requisites
--------------

*(You might already have it)*

Xcode command line tools
~~~~~~~~~~~~~~~~~~~~~~~~

::

   xcode-select --install

Homebrew
~~~~~~~~

::

   ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

Eclipse
~~~~~~~

*(Optional)*::

   brew install Caskroom/cask/eclipse-cpp

Build tools
-----------

Required for the makefile build system::

   brew install binutils coreutils automake wget gawk libtool gettext gperf grep
   export PATH="/usr/local/opt/gnu-sed/libexec/gnubin:$PATH"

Install gnu-sed and make sure that it is the default ``sed`` command line application::

   brew install gnu-sed --with-default-names

If you have already installed ``gnu-sed`` but it is not the default one,
then make sure to uninstall it and install it again with the correct options::

   brew uninstall gnu-sed
   brew install gnu-sed --with-default-names

ESP8266 Toolchain
-----------------

We pull this in from the `SmingTools <https://github.com/SmingHub/SmingTools/releases>`__ repository::

   cd ~/
   curl -L -O https://github.com/SmingHub/SmingTools/releases/download/1.0/esp-open-sdk-macosx.tar.gz
   sudo mkdir -p /opt/
   sudo tar -zxf esp-open-sdk-macosx.tar.gz -C /opt/
   sudo chmod -R 775 /opt/esp-open-sdk

You can also build it yourself
`with Homebrew <https://github.com/pfalcon/esp-open-sdk#macos>`__ or
`with MacPorts <http://www.esp8266.com/wiki/doku.php?id=setup-osx-compiler-esp8266>`__.

Get Sming Core
--------------

Clone from the `Sming <https://github.com/SmingHub/Sming>`__ repository::

   cd <your-favourite-development-folder>/
   git clone https://github.com/SmingHub/Sming.git
   cd Sming

.. warning::

   Do NOT use the --recursive option for the command above.
   Our build mechanism will take care to get the third-party sources and patch them, if needed.

You will get a copy of our `develop` branch which intended for developers.
It is the one where all new cool (unstable) features are landing.

If you want to use our stable branch then use the master branch::

   git checkout origin/master

Finally, set the :envvar:`SMING_HOME` environment variable to point to <your-favourite-development-folder>/Sming/Sming::

   export SMING_HOME=`pwd`

Environment Variables
---------------------

Open with a text editor the ``.profile`` file in your home directory, and add these lines::

   export ESP_HOME=/opt/esp-open-sdk
   export SMING_HOME=<your-favourite-development-folder>/Sming/Sming

Make sure to replace ``<your-favourite-development-folder>`` in the
command above with the actual directory on your local disk.

(Optional step)
~~~~~~~~~~~~~~~

*(used by Make and Eclipse - make sure to quit Eclipse first)*

If you installed Eclipse manually, substitute
``/opt/homebrew-cask/Caskroom/eclipse-cpp/4.5.1/Eclipse.app`` to
``/Applications/Eclipse/eclipse.app``::

   sudo /usr/libexec/PlistBuddy -c "Add :LSEnvironment:ESP_HOME string '/opt/esp-open-sdk'" /opt/homebrew-cask/Caskroom/eclipse-cpp/4.5.1/Eclipse.app/Contents/Info.plist
   sudo /usr/libexec/PlistBuddy -c "Add :LSEnvironment:SMING_HOME string '/opt/sming/Sming'" /opt/homebrew-cask/Caskroom/eclipse-cpp/4.5.1/Eclipse.app/Contents/Info.plist
   sudo /System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister -v -f /opt/homebrew-cask/Caskroom/eclipse-cpp/4.5.1/Eclipse.app

Using Eclipse on Mac
--------------------

If you are using Eclipse to build the samples you need to make sure the
path is set correctly for the make process.

Your project must also be configured to use the correct serial port
for your ESP8266. You can change it like this::

   make COM_PORT=/dev/tty.usbserial

Next, ensure that you can build the :sample:`Basic_Blink` from a terminal window::

   cd $SMING_HOME/samples/Basic_Blink
   make

This will also build the required framework components, so may take a few minutes.
If this works without errors then type ``echo $PATH`` and copy the resulting path
to the clipboard.

Now fire up Eclipse and go to

*Eclipse ==> Preferences ==> C/C++ ==> Build ==> Environment*

and add a new variable PATH. Paste in the path saved from the terminal
session above. You can also add :envvar:`SMING_HOME` and :envvar:`ESP_HOME` variables here
the same way as you set in the export commands above which will then be
set for all the projects.

The standard make files use ``miniterm.py`` to provide a serial :component:`terminal` for
debugging the ESP8266. Miniterm does not work inside Eclipse so you
should disable it like this::

   make KILL_TERM= TERMINAL=

This will prevent Eclipse from trying to launch miniterm and throwing an
error about Inappropriate ioctl for device.

You can use the built in terminal in Eclipse Oxygen by adding it using

*Window ==> Show View ==> Terminal*

then setting terminal type to ``Serial`` and setting the port to the port
the ESP8266 is connected to. Remember to disconnect before tying to
re-flash the device though.

Compile Sming Examples
----------------------

See :doc:`/samples` for a list of all examples provided with Sming::

   cd $SMING_HOME/../samples/

If you want to test some of the examples, try this::

   cd $SMING_HOME/../samples
   cd Basic_Blink
   make
   # The command below will upload the sample code to your ESP8266 device
   make flash

Next steps
----------

Proceed to :doc:`config`.
