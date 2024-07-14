Manual Windows Installation
===========================

.. highlight:: batch

These notes are provided for informational purposes.

Packages
--------

These packages must be installed on the system and accessible via system PATH. This is handled by the ``tools\choco-install.cmd`` script, which also installs the chocolatey package manager.

git
   `GIT <https://git-scm.com/>`__ CLI client.

   Please configure after installation to leave line-endings intact or else patching will fail::

      git config --global core.autocrlf input

Python
   `Python <https://www.python.org/>`__ version 3.8 or newer.

CMake
   `CMake <https://cmake.org/>`__.

   Required to build some Components, also for Host mode.

Ninja
   `https://ninja-build.org/`__.

   Required to build some Components, also for Host mode.

MinGW
   `MinGW <https://osdn.net/projects/mingw/>`__ 32-bit.

   This is **NOT** a choco package, but installed via the ``mingw-install.cmd`` script. The script updates the system ``PATH``, and can be checked by running::

      where make.exe

   The output should show only one result::

      "C:\MinGW\msys\1.0\bin\make.exe"

If you have specific installation requirements it is usually simpler to customise the installation scripts.

SMING_TOOLS_DIR
   This defaults to ``C:\tools`` and determines the root location for all toolchains.

   If you want to install to a different directory, set this globally. For example::

      setx SMING_TOOLS_DIR C:\opt

   When ``Tools\export.cmd`` is next run the paths will change to reflect this value.

   The ``Tools\install.cmd`` script will also install to this new location.


MinGW
-----

Code built for target devices such as the ESP8266 use a separate toolchain, but the Host Emulator and Components such as SPIFFS require additional tools which are built as Windows executable applications.

MinGW provides a (mostly) POSIX-compliant development environment for Windows, including GNU Make and various other command-line tools.

.. note::

   There are two versions of MinGW.

   `MinGW <https://osdn.net/projects/mingw/>`__ is the original, and the version currently used with Sming. It does not appear to be maintained any more.

   `MinGW-w64 <http://mingw-w64.org/>`__ was forked from MinGW in 2007 *in order to provide support for 64 bits and new APIs*. More recent toolchains are available but at present this is not supported by Sming.

To find out if you already have GCC on your system, type::

   where gcc

If it shows ``C:\MinGW\bin\gcc.exe`` then you have a standard MinGW installation. Check the gcc version::

   gcc --version

The current version is 9.2.0. Refer to the ``tools\mingw-install.cmd`` script for installation details.

Note that the system PATH must be updated as follows::

      SETX PATH "C:\mingw\bin;C:\mingw\msys\1.0\bin;%PATH%"

Make sure it is in this exact order. This is to ensure that ``make`` and ``sh`` are run from the correct locations.

You will need to restart your command prompt (and Eclipse, if already running) for these changes to take effect.

.. important::

   The above ``setx`` command, and its sibling ``set``, are both flawed in that the path will be truncated to 4095 characters. For this reason, the installer uses powershell to update the path which does not have this issue. The script includes extra checks to ensure paths are not duplicated.


Paths and locations
-------------------

You can put Sming anywhere convenient.
The ``tools\export.cmd`` script will ensure that ``SMING_HOME`` is set correctly.

Please observe the following precautions::

There must be **no spaces** in the path
   GNU make cannot handle spaces.

Be consistent with case
   Whilst Windows filenames are not (by default) case-sensitive, the compiler tools are.   
   Please take care to type paths exactly as shown.

Don't use excessively long paths
   This can avoid some weird issues if paths become too long.
   If you encounter problems, you can always remap paths using ``subst``.
   For example::

      subst S: C:\Users\Kevin\Documents\Development\Embedded


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
