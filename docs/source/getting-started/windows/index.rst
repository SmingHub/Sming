Windows Installation
====================

.. highlight:: batch

This page describes how to install the Sming framework and required tools for Windows.
See :doc:`manual` for details of what gets installed.

Sming requires GNU make and associated tools which are not well-supported by Windows,
so requires MinGW.
You may have a better experience using the :doc:`Windows Subsystem for Linux <wsl>`.

.. toctree::
   :hidden:
   :glob:

   *

Quick Install
-------------

1. Install Sming with essential tools

Open an *administrative* **cmd.exe** command prompt and navigate to a directory of your choosing.
The Sming framework will be cloned into a subdirectory named ``sming``.

Paste the following text press enter::

   curl -LO https://raw.githubusercontent.com/SmingHub/Sming/develop/Tools/choco-install.cmd && choco-install.cmd

This will install the `Chocolatey <https://chocolatey.org>`__ package manager on your system,
then use it to install the required tools such as `GIT <https://git-scm.com/>`.
The Sming framework is then installed 

We will make use of the `Chocolatey <https://chocolatey.org>`__ package manager to install these,
if not already present on your system.

.. important::

   After installation, please close the administrative command prompt and open a new, regular command shell.

   This ensures that environment variables are set correctly.

   It is also inadvisable to continue running with elevated privileges.


2. Install toolchains

   To install toolchains for all supported architectures, type this command::

      sming\tools\install all

   If you want to save disk space (and installation time) then you can select which tools to install.
   Get a list of available options like this::

      sming\tools\install


3. Set environment variables

   This command must be run every time a new command shell is opened::

      sming\tools\export

   This sets important environment variables such as ``SMING_HOME``, but **only** for the current session.

   To persist these via the Windows registry::

      sming\tools\export -persist

   This means they do not need to be set every time a command prompt is opened, and will be seen by
   integrated development environments such as Eclipse or VSCode without any further configuration.


If you followed and executed carefully the steps above Sming should be installed and configured.
You can scroll down to `Build Basic_Blink`_ to check the installation.


Optional step: Re-installation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The installer scripts do not overwrite existing installations.
If something is broken or you want to upgrade, delete the appropriate directory before running the installation.

For example, to reinstall the ESP8266 toolchain::

   rmdir /s c:\tools\esp-quick-toolchain
   %SMING_HOME%\..\tools\install


Build Basic_Blink
-----------------

To check the installation, open a command prompt and type these commands::

   cd c:\tools\sming
   tools\export
   cd samples\Basic_Blink
   make

The project should build without error.

If you want to try out the Host emulator, do this::

   make -j SMING_ARCH=Host

For build options::

   make help


Next steps
----------

Proceed to :doc:`../config`.
