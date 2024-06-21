Mac-OS Installation
===================

.. highlight:: bash


Quick Install
-------------

1. Install command-line development tools::

   xcode-select --install

2. Check if ``homebrew`` is installed::

      brew

   If not, install it::

      /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

   Check https://brew.sh/ if there's any issues here.

3. Prepare installation directory

   Let's use /opt as the main directory for tools and Sming.

   Regular users may not have access to */opt*, so do this::

      sudo chown $USER /opt

   (alternatively, use a different directory).

4. Fetch the Sming repository::

      git clone https://github.com/SmingHub/Sming /opt/sming

   If using a directory other than ``/opt`` then you'll need to edit ``sming/Tools/export.sh`` before proceeding.

5. Run the installer::

      source /opt/sming/Tools/install.sh all

   If you want to save disk space then you can select which tools to install.
   Get a list of available options like this::

      /opt/sming/Tools/install.sh


Vscode
------

See https://code.visualstudio.com/.


Eclipse
-------

*(Optional)*::

   brew install Caskroom/cask/eclipse-cpp


.. note::

   The following information may be out of date.


Environment Variables
---------------------

Open with a text editor the ``.profile`` file in your home directory, and add these lines::

   export ESP_HOME=/opt/esp-quick-toolchain
   export SMING_HOME=opt/Sming/Sming

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

   cd $SMING_HOME/../samples/Basic_Blink
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

Proceed to :doc:`../config`.
