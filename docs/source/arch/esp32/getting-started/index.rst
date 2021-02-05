Getting Started: Esp32
======================

Requirements
------------

Pre-requisites::

   sudo apt-get install ninja-build ccache

The ESP-IDF v4.1 and corresponding toolchains are requied to build for the ESP32 architecture.

Linux:

.. code-block:: bash

   source $(SMING_HOME)/Arch/Esp32/Tools/install.sh

   export IDF_PATH=/opt/esp-idf
   export IDF_TOOLS_PATH=/opt/esp32
   export ESP32_PYTHON_PATH=/usr/bin
   git clone -b release/v4.1 https://github.com/espressif/esp-idf.git /opt/esp-idf
   mkdir -p $IDF_TOOLS_PATH
   SMINGTOOLS=https://github.com/SmingHub/SmingTools/releases/download/1.0
   ESPTOOLS=esp32-tools-linux-4.1.zip
   wget $SMINGTOOLS/$ESPTOOLS
   unzip $ESPTOOLS -d $IDF_TOOLS_PATH/dist
   python3 $IDF_PATH/tools/idf_tools.py install
   python3 -m pip install -r $IDF_PATH/requirements.txt


Windows:

.. code-block:: batch

   set IDF_PATH=c:\tools\esp-idf
   set IDF_TOOLS_PATH=c:\tools\esp32
   set ESP32_PYTHON_PATH=c:\Python39
   git clone -b release/v4.1 https://github.com/espressif/esp-idf.git %IDF_PATH%
   mkdir %IDF_TOOLS_PATH%
   SMINGTOOLS=https://github.com/SmingHub/SmingTools/releases/download/1.0
   ESPTOOLS=esp32-tools-windows-4.1.7z
   curl -LO %SMINGTOOLS%/%ESPTOOLS%
   7z -o%IDF_TOOLS_PATH%\dist x %ESPTOOLS%
   python3 %IDF_PATH%\tools\idf_tools.py install
   python3 -m pip install -r %IDF_PATH%\requirements.txt


For further information about installation and other issues, see
`ESP-IDF documentation <https://docs.espressif.com/projects/esp-idf/en/v4.1/get-started/index.html#installation-step-by-step>`__.


Building
--------

Make sure that the IDF_PATH environmental variable is set. On Linux you can use for example the following command::

    export IDF_PATH=/opt/esp-idf

Note that you do not need to run ``IDF export.sh`` as this is handled by the Sming build system.
This simplifies use within the Eclipse IDE as there is only one environment variable to set.

Build the framework and application as usual, specifying :envvar:`SMING_ARCH` =Esp32. For example::

   cd $SMING_HOME/../samples/Basic_Serial
   make SMING_ARCH=Esp32

This builds the application. Once built the application needs to be flashed on a real Esp32 microcontroller to run.
Flashing can be done using the following command::

  make flash


.. note::

   If you have an ESP32-S2 device you'll need to change :envvar:`ESP_VARIANT`::
   
      make ESP_VARIANT=esp32s2


See :component-esp32:`esp32` for further details.
