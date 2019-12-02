ESP Quick Toolchain
===================

Introduction
------------

In Sming 4.0.1 support was added for the `ESP Quick Toolchain <https://github.com/earlephilhower/esp-quick-toolchain>`__
which provides support for GCC 9.2.0.

It also updates the runtime libraries (`NewLib <https://github.com/earlephilhower/newlib-xtensa>`__)
to version 2.2 with integrated PROGMEM handling code.

See :pull-request:`1825` for further details.

The new toolchain is consistent across development platforms and adds support for the latest compiler features,
as discussed in `What are the new features in C++17? <https://stackoverflow.com/questions/38060436/what-are-the-new-features-in-c17>`__.

Installation
------------

The toolchains are currently at pre-release, available `here <https://github.com/earlephilhower/esp-quick-toolchain/releases>`__.
Download links as follows:

- Linux 32-bit: `i686-linux-gnu.xtensa-lx106-elf-dd9f9a2.1569802152.tar.gz <https://github.com/earlephilhower/esp-quick-toolchain/releases/download/3.0.0-gnu2/i686-linux-gnu.xtensa-lx106-elf-dd9f9a2.1569802152.tar.gz>`__
- Linux 64-bit: `x86_64-linux-gnu.xtensa-lx106-elf-dd9f9a2.1569802152.tar.gz <https://github.com/earlephilhower/esp-quick-toolchain/releases/download/3.0.0-gnu2/x86_64-linux-gnu.xtensa-lx106-elf-dd9f9a2.1569802152.tar.gz>`__
- Windows: `x86_64-w64-mingw32.xtensa-lx106-elf-dd9f9a2.1569802152.zip <https://github.com/earlephilhower/esp-quick-toolchain/releases/download/3.0.0-gnu2/x86_64-w64-mingw32.xtensa-lx106-elf-dd9f9a2.1569802152.zip>`__

Extract the toolchain to a suitable location, such as:

* ``/opt/esp-quick-toolchain``
* ``C:\tools\esp-quick-toolchain``

and set :envvar:`ESP_HOME` accordingly.
