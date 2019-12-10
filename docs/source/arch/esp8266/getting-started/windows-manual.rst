Manual Windows Installation
===========================

.. highlight:: batch

MinGW
-----

Code built for the ESP8266 uses a separate Espressif compiler, but Components such as SPIFFS
require additional tools which are built as Windows executable applications.

MinGW provides a (mostly) POSIX-compliant development environment for Windows, including GNU Make and
various other command-line tools.

.. note::

   There are two versions of MinGW.

   `MinGW <http://mingw.org/>`__ is the original, and the version recommended for use with Sming.

   `MinGW-w64 <http://mingw-w64.org/>`__ was forked from MinGW in 2007 *in order to provide support
   for 64 bits and new APIs*. (Which we don't need!)


To find out if you already have GCC on your system, type::

   where gcc

If it shows ``C:\MinGW\bin\gcc.exe`` then you have a standard MinGW installation. Check the gcc version::

   gcc --version

The current version is 8.2.0. You can upgrade by renaming or removing your existing installation then
following these instructions.

Fast install
~~~~~~~~~~~~

1. Download `MinGW.7z <https://github.com/SmingHub/SmingTools/releases/download/1.0/MinGW.7z>`__ from the SmingTools repository.

2. Unzip to default location::

      7z -oC:\ x MinGW.7z

   .. note::
   
      You can obtain 7-zip from https://www.7-zip.org/.

3. Update :envvar:`PATH` environment variable::

      SETX PATH "C:\mingw\bin;C:\mingw\msys\1.0\bin;%PATH%"

   Make sure it is in this exact order. If you have Cygwin installed make sure the above entries appear first.

   You will need to restart your command prompt (and Eclipse, if already running) for these changes to take effect.
   

Alternative install
~~~~~~~~~~~~~~~~~~~

To install from the original MinGW source repository:

1. Get the `MingGW Setup <https://osdn.net/projects/mingw/downloads/68260/mingw-get-setup.exe>`__ and run it.
   This will create the ``C:\MinGW`` directory with minimal content.

2. Set PATH environment variable as above.

3. Install required MinGW packages::

      mingw-get install mingw32-base-bin mingw-developer-toolkit-bin mingw32-gcc-g++-bin mingw32-pthreads-w32-dev mingw32-libmingwex

   Note that you can upgrade packages using::
   
      mingw-get update
      mingw-get upgrade

   However this will not upgrade a 6.3.0 installation to 8.2.0.


Install ESP8266 Toolchain
-------------------------

1. Download toolchain `esp-udk-win32.7z <https://github.com/SmingHub/SmingTools/releases/download/1.0/esp-udk-win32.7z>`__.

2. Unzip to default location::

      7z -oC:\Espressif x esp-udk-win32.7z

3. Set :envvar:`ESP_HOME` environment variable::

      SETX ESP_HOME C:\Espressif

.. note::
   There is NO trailing slash on the path!
   
   If you want to set environment variables system-wide, append /M to the command.
   You'll need to do this from an administrative command prompt.


Install GIT
-----------

This is required to fetch and update Sming code from its repository.

1. Install command-line `GIT <https://git-scm.com/downloads>`__ client.

These steps are optional, but highly recommended:

2. Install Graphical client `Git Extensions <https://gitextensions.github.io/>`__.
3. Create an account at https://github.com. This will allow you to participate in discussions, raise issues
   and, if you like, :doc:`/contribute/index` to the framework!


Download Sming
--------------

1. You can put Sming anywhere convenient, provided there are **no spaces** in the path!
   For example, *C:\\tools\\sming*::

      mkdir C:\tools\sming
      cd /d C:\tools\sming

2. To obtain the latest develop code with all the latest features and fixes::

      git clone https://github.com/SmingHub/Sming

   To obtain the latest release::

      git clone https://github.com/SmingHub/Sming --branch master

3. Set :envvar:`SMING_HOME` environment variable::

      SETX SMING_HOME C:\tools\sming\Sming

   Note: there is NO trailing slash on the path!
   
.. note::
   Whilst Windows filenames are not (by default) case-sensitive, the compiler tools are.
   
   Please take care to type paths exactly as shown.

At this stage you should be able to build a sample::

   cd samples\Basic_Blink
   make -j

If you want to try out the Host emulator, do this::

   make -j SMING_ARCH=Host

For build options::

   make help


Install Eclipse IDE
-------------------

Whilst building and configuring your application is generally easier and faster using the command prompt,
developing and debugging code is greatly simplified using an Integrated Development Environment (IDE).

1. Install `Java Runtime Environment <https://www.oracle.com/technetwork/java/javase/downloads/>`__.
2. Install `Eclipse <http://eclipse.org/downloads/packages/>`__ IDE for C++ Developers.
3. Start Eclipse IDE. When prompted, enter ``C:\tools\sming`` as the workspace path.
4. Select *File -> Import -> General -> Existing Project* into Workspace.
   In the line *Select root directory*, select the directory ``C:\tools\sming\Sming`` and import everything.
5. Go have a cup of coffee while Eclipse scans all the source code. It can take a while!
6. To build a project, right-click and select *Build project*. Alternatively, select the project and press F9.


Eclipse IDE variables
---------------------

The only variable you should need to set within Eclipse is :envvar:`SMING_HOME`.
You can set this within the Eclipse IDE via *Window > Preferences -> C/C++ > Build > Environment*.

If you set this via global environment variable *before* starting Eclipse then this step is not necessary.

.. note::
   Variables set within the IDE won't be accessible in other Eclipse sessions or the command prompt.

All other configuration should be done either in your project's *component.mk* file or via command line.

For example, to switch to a Host emulator build, do this::

   make SMING_ARCH=Host list-config

This also displays the current configuration settings. Whether you build from command line or Eclipse,
the same settings will be used.


Next steps
----------

Proceed to :doc:`config`.
