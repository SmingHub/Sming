Windows Installation
====================

.. highlight:: powershell

This page describes how to install the required tools and obtain the current
release version of Sming using the `Chocolatey <https://chocolatey.org>`__ package manager.

Use :doc:`windows-manual` for control over installation locations. 


Install Chocolatey
------------------

Open an *administrative* **cmd.exe** command prompt and paste the text from the box below and press enter::

   @powershell -NoProfile -ExecutionPolicy unrestricted -Command "iex ((new-object net.webclient).DownloadString('https://chocolatey.org/install.ps1'))" && SET PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin

Upgrade .NET
------------

Choco requires a recent version of .NET in order to be able to access HTTPS links.
To check if you need to upgrade, type the following command::

   choco --v

If you see a warning similar to the one below then make sure to upgrade your .NET version:

.. code-block:: text

   Choco prefers to use TLS v1.2 if it is available, but this client is running on .NET 4.3,
   which uses an older SSL. It's using TLS 1.0 or earlier, which makes it susceptible to BEAST
   and also doesn't implement the 1/n-1 record splitting mitigation for Cipher-Block Chaining.
   Upgrade to at least .NET 4.5 at your earliest convenience.

You can upgrade the .NET version from this
`direct link <https://www.microsoft.com/en-us/download/details.aspx?id=55170>`__
or search in internet for “.NET upgrade”.


Install packages
----------------

First, tell Chocolatey about the Sming package repository::

   choco sources add -name smingrepo -source 'https://www.myget.org/F/sming/'

You can find the installer sources at `https://github.com/slaff/chocolatey-packages`.

Now, open an **administrative** command prompt and run::

   choco install -y sming

This will install the following packages: the names are given should you wish to install them individually.

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


.. important::

   After installation, please close the administrative command prompt and open a new, regular command shell.

   This ensures that environment variables are set correctly.

   It is also inadvisable to continue running with elevated privileges.


Build Basic_Blink
-----------------

To check the installation, open a command prompt and type these commands::

   cd %SMING_HOME%\..\samples\Basic_Blink
   make

The project should build without error.
   

Update Sming
------------

Sming is very dynamic and updates are usually announced in gitter. The command below will get for you the latest ``stable`` release::

   cd %SMING_HOME%
   git pull

If you would like access to the latest features and bug-fixes, use the ``develop`` branch::

   cd %SMING_HOME%
   git checkout develop


Force Reinstall Sming
---------------------

In case something is broken, this will perform a forced re-install of all packages::

   rmdir /s /q c:\tools\sming
   choco install sming -y -f -x



Next steps
----------

Proceed to :doc:`config`.

