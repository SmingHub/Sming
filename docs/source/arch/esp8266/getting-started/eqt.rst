ESP Quick Toolchain
===================

Introduction
------------

In Sming 4.0.1 support was added for the `ESP Quick Toolchain <https://github.com/earlephilhower/esp-quick-toolchain>`__.

At time of writing the current release is 
`3.0.0-newlib4.0.0-gnu20 <https://github.com/earlephilhower/esp-quick-toolchain/releases/tag/3.0.0-newlib4.0.0-gnu20>`__
for `GCC 10.2 <https://www.gnu.org/software/gcc/gcc-10>`__.

This also updates the runtime libraries (`NewLib <https://github.com/earlephilhower/newlib-xtensa>`__)
to version 2.2 with integrated PROGMEM handling code.

The new toolchain is consistent across development platforms and adds support for the latest compiler features,
as discussed in `What are the new features in C++17? <https://stackoverflow.com/questions/38060436/what-are-the-new-features-in-c17>`__.

Installation
------------

The toolchains are currently at pre-release, available `here <https://github.com/earlephilhower/esp-quick-toolchain/releases>`__.

Tested versions are available at `SmingTools <https://github.com/SmingHub/SmingTools>`.

Extract the toolchain to a suitable location, such as:

* ``/opt/esp-quick-toolchain``
* ``C:\tools\esp-quick-toolchain``

and set :envvar:`ESP_HOME` accordingly.
