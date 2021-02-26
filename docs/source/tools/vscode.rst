Using with MS Visual Studio Code
================================

.. highlight:: bash

Microsoft `Visual Studio Code <https://code.visualstudio.com/>`__ is a free (as in
"free beer") and Open Source code editor for Windows, Linux and Mac.

For easier integration make sure you have both :envvar:`ESP_HOME` and
:envvar:`SMING_HOME` exported in your working environment.


Software involved
-----------------

-  `Visual Studio Code <https://code.visualstudio.com/>`__
-  `C/C++ extension <https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools>`__


Installation
------------

-  Install VS Code, extensions and tools
-  Navigate to project folder and create configuration as described below
-  Open workspace. If vscode is in your system path you can do this::

      code .


Configuration
-------------

One of the strengths of vscode is the use of well-documented configuration files.
You can find comprehensive documentation for these online.

However, setting these up is time-consuming so the build system can create them for you.
The vscode workspace root directory is your project directory.

Change to your project directory (e.g. ``samples/Basic_Blink``) and run these commands::

   make ide-vscode SMING_ARCH=Esp8266
   make ide-vscode SMING_ARCH=Host

Now open the workspace in vscode, and open a source file (.c, .cpp, .h).
You should now be able to select the architecture from the icon in the bottom-right corner:

.. figure:: vscode1.png

   VS Code language selection

A selection of tasks are provided which you can view via ``Terminal`` -> ``Run Task``.

To debug your application, follow these steps:

-  Select the appropriate architecture (e.g. Host, Esp8266)
-  Select ``Terminal`` -> ``Run Task`` -> ``Full rebuild (with debugging)``
-  Confirm that the baud rate (:envvar:`COM_SPEED_GDB`) and port (:envvar:`COM_PORT_GDB`) are
   set correctly::

      make gdb SMING_ARCH=Esp8266 COM_PORT_GDB=/dev/ttyUSB0 COM_SPEED_GDB=115200

-  Update the vscode configuration::

      make ide-vscode

-  In vscode, select the require 'Run' task:

.. figure:: vscode2.png

   VS Code debug selection


Manual configuration changes
----------------------------

When you run ``make ide-vscode`` the configuration files are actually generated using a python script
``Tools/vscode/setup.py``. Configuration variables are passed from the project makefile.

If you make any changes to the configuration files, please note the following behaviour:

-  The ``Host``, ``Esp32`` or ``Esp8266`` intellisense settings will be overwritten.
-  The ``Esp8266 GDB`` and ``Host GDB`` launch configurations will be overwritten
-  The ``sming.code-workspace`` and ``.vscode/tasks.json`` files will be created if they do not already exist.
   To re-create these they must first be deleted.

Ideally the vscode configuration files should not need to be kept under configuration control,
but generated when required.

Some settings are necessarily configured via the ``setup.py`` script, however most settings can
be changed by editing the files in ``Tools/vscode/template``.

If you do this, remember to keep a copy as they'll be overwritten otherwise.

And, please consider contributing any changes or suggestions to the community!


Known issues / features
-----------------------

-  The vscode configuration files are only updated when you manually run ``make ide-vscode``.
   If you update change critical build variables or add/remove Components to your project,
   you may need to run it again to update them.
-  When running ``make ide-vscode``, comments in the configuration files will be discarded.
-  ``make ide-vscode`` may overwrite parts of your configuration: be warned!
-  When debugging for esp8266 output in the console is not formatted correctly.
   Lines appear with @ in front of them.
-  A debugging configuration is not currently provided for ESP32.
