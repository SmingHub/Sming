Windows Installation
====================

.. highlight:: powershell

This page describes how to install the required tools and obtain the current
release version of Sming using the `Chocolatey <https://chocolatey.org>`__ package manager.

See also:

.. toctree::
   :titlesonly:
   :maxdepth: 1

   manual
   wsl


Quick Install
-------------

Open an *administrative* **cmd.exe** command prompt and paste the text from the box below and press enter::

   curl -LO https://raw.githubusercontent.com/SmingHub/Sming/develop/Tools/choco-install.cmd && choco-install.cmd


.. important::

   After installation, please close the administrative command prompt and open a new, regular command shell.

   This ensures that environment variables are set correctly.

   It is also inadvisable to continue running with elevated privileges.


The installer uses the latest **develop** branch.
The command below will get for you the latest ``stable`` release::

   cd %SMING_HOME%
   git checkout master
   git pull


In case something is broken, this will perform a forced re-install of all packages::

   rmdir /s /q c:\tools\sming
   choco install sming -y -f -x



Packages
--------

You can find the installer sources at https://github.com/slaff/chocolatey-packages.
Packages are as follows:

git
   `GIT <https://git-scm.com/>`__ CLI client.

   Please configure after installation to leave line-endings intact or else patching will fail::

      git config --global core.autocrlf input

python
   `Python <https://www.python.org/>`__ version 3.

cmake
   `CMake <https://cmake.org/>`__.

   Required to build some Components, also for Host mode.

mingw
   `MinGW <http://www.mingw.org/>`__ 32-bit.

   The installer updates the system ``PATH`` but please check by running::

      where make.exe

   The output should show only one result::

      "C:\MinGW\msys\1.0\bin\make.exe"

esp8266-eqt
   `ESP Quick Toolchain <https://github.com/earlephilhower/esp-quick-toolchain/>`__.

   Sets a system-wide :envvar:`ESP_HOME` variable.

sming.core
   Latest stable version of `Sming <https://github.com/SmingHub/Sming/tree/master>`__.

   Sets a system-wide :envvar:`SMING_HOME` environment variable.

Note that setting `SMING_HOME` and `ESP_HOME` as system-wide variables means they do
not need to be set every time a command prompt is opened, and will be seen by eclipse
without any further configuration.


Build Basic_Blink
-----------------

To check the installation, open a command prompt and type these commands::

   cd %SMING_HOME%\..\samples\Basic_Blink
   make

The project should build without error.
   

Next steps
----------

Proceed to :doc:`config`.

