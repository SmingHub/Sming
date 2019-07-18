===========================
Manual Windows Installation
===========================

This is a **DEPRECATED** way of installing Sming. It will not be
supported in the next versions.

.. attention::
   Please consider using a simple way of installing Sming
   via :doc:`Chocolatey <windows>`. Package sources are available as well.

If you still want to install it manually, please make sure the
following steps are taken:

1. Install Unofficial Development Kit
-------------------------------------

1. Install `Universial Development Kit <http://programs74.ru/get.php?file=EspressifESP8266DevKit>`__.
2. Install `Java Runtime Environment <http://www.oracle.com/technetwork/java/javase/downloads/jre8-downloads-2133155.html>`__.
3. Install `Eclipse Luna <http://eclipse.org/downloads/packages/eclipse-ide-cc-developers/lunasr2>`__.
   Unpack the archive to ``c:\Eclipse``.
4. Install `MingGW <http://sourceforge.net/projects/mingw/files/Installer/>`__.
   Run mingw-get-setup.exe. Add install support for the graphical
   user interface. Install to ``c:\MinGW``.
5. Install `additional modules for MinGW <http://programs74.ru/get.php?file=EspressifESP8266DevKitAddon>`__.
   Run ``install-mingw-package.bat``
6. Start the Eclipse Luna from ``c:\Eclipse\eclipse.exe``
7. In Eclipse, select File -> Import -> General -> Existing Project into
   Workspace. In the line `Select root directory`, select the directory
   ``C:\Espressif\examples`` and import work projects. Further, the right
   to select the Make Target project, such as hello-world and run the
   target All the assembly, while in the console window should display the
   progress of the build. To select the target firmware flash.

Install additional mingw packages
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

   C:\MinGW\bin\mingw-get install "msys-make"

Create directory link for compatibility building (with admin permissions)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``mklink /d c:\Espressif\sdk c:\Espressif\ESP8266_SDK``

2. Set Environment Variables (requires Eclipse/cmd restart)
-----------------------------------------------------------

Windows System **PATH** variable should contain a path to ``c:\mingw\bin`` and ``c:\mingw\msys\1.0\bin``:

::

   SETX PATH /M C:\mingw\bin;C:\mingw\msys\1.0\bin;%PATH%

*Make sure it is in this exact order. This step is also required for some old sming installations.*

Also make sure, mingw64/bin is path before cygwin bin in %PATH% environmental variable in windows.


3. Make sure to set SMING_HOME and ESP_HOME environment variables
-----------------------------------------------------------------

::

   SETX SMING_HOME c:\tools\Sming\Sming
   SETX ESP_HOME c:\Espressif

*Please make sure there are no trailing slashes at the end of either path!*


Eclipse IDE variables
---------------------

You can manually add variables into Eclipse IDE: Window > Preferences -> C/C++ > Build > Environment.

*It won’t be accessible outside Eclipse IDE*
