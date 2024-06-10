Linux Installation
==================

.. highlight:: bash

Quick Install
-------------

Debian (Ubuntu) and Fedora systems can use the scripted installer.

1. Prepare installation directory

   Let's use /opt as the main directory for tools and Sming.
   
   Regular users may not have access to */opt*, so do this::
   
      sudo chown $USER:$USER /opt
   
   (alternatively, use a different directory).


2. Install GIT

   **Debian**::
   
      sudo apt-get install -y git
   
   **Fedora**::
   
      dnf install -y git
   

3. Fetch the Sming repository::

      git clone https://github.com/SmingHub/Sming /opt/sming

4. Run the installer::

      source /opt/sming/Tools/install.sh all


If you want to save disk space then you can select which tools to install.
Get a list of available options like this::

   /opt/sming/Tools/install.sh

Install locations can be customised by setting environment variables before
running the install. Certain variables should also be set globally.
See :doc:`/getting-started/config` for details.

If you want to use the stable (release) branch::

   cd /opt/sming
   git checkout master
   git pull


Build a ‘Basic Blink’ example
-----------------------------

Change to the application directory and build::

   cd $SMING_HOME/../samples/Basic_Blink
   make

Flash to your device (using default serial port)::

   cd $SMING_HOME/../samples/Basic_Blink
   make flash


Next steps
----------

Proceed to :doc:`../config`.
