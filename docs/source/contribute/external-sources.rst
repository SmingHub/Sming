***********************
Adding External Sources
***********************

This document describes the recommended way to add source code from
other ``git`` repositories.

Introduction
============

In Sming we have source code from other repositories:

* `rboot <https://github.com/raburton/rboot>`__
* `esp-gdbstub <https://github.com/espressif/esp-gdbstub>`__
* `spiffs <https://github.com/pellepl/spiffs>`__

etc..

Having local copies of those modules brings some disadvantages with it:

1. We cannot easily update the local copy from the original source code.
2. We cannot easily send improvements to those projects once we make
   local changes in our local copy.

Therefore we started using ``git submodules``. For the sake of brevity
``git submodules`` will be called ``submodules`` in this document.
Submodules allow us to fetch the source code from the respective
repositories and eliminate the disadvantages that local copies have. In
addition, if we need local modifications than we can keep a .patch file
and apply it after fetching the submodule code. This decreased our
efforts and allowed us to use the latest versions of those third-party
projects.

Fetching Source Code
====================

Without git repository
----------------------

If the source code does not have a publicly accessible git repository
then the source code needs to be copied locally. It should reside in a
folder inside the ``Sming/third-party/`` folder. All local changes need
to be applied directly to the local copy.

With git repository
-------------------

In the cases where a library or dependent component has a public git
repository we should add it in the following way. First we need to add
the external repository as submodule. For example if we want to use the
`pwm <https://github.com/StefanBruens/ESP8266_new_pwm%20we>`__ submodule
we can do the following

::

   cd <Sming-root-folder>/
   git submodule add  https://github.com/StefanBruens/ESP8266_new_pwm.git Sming/third-party/pwm

The command above instructs ``git`` to register the repository
``https://github.com/StefanBruens/ESP8266_new_pwm.git`` as a submodule
that will be present in our local source code in the folder
``Sming/third-party/pwm``. All submodules should be pointing to a
directory that is sub-directory of the ``Sming/third-party`` folder.
This way every developer can use the existing mechanism to fetch and
link these modules to the rest of the project.

Applying Patches
================

If the module needs local modifications to work with our project then we
should add all needed changes in a patch file. For example for the
``pwm`` module all changes should be saved in
``Sming/third-party/.patches/pwm.patch``. If the module that we patch is
called ``esp-gdbstub`` then the patch file should be
``Sming/third-party/.patches/esp-gdbstub.patch``. Please, use this
naming for consistency.

Link external sources in Sming
==============================

Once we are ready with the submodules we need to instruct Sming that a
new source code is present and how to use it. Most of the changes should
be done in the /Sming/Makefile. First we have to modify the
``THIRD_PARTY_DATA`` variable in the Makefile. For example if the
variable has the following data

``THIRD_PARTY_DATA = third-party/rboot/Makefile``

we should add an existing file from the new submodule. In the case of
the ``pwm`` submodule an existing file in it is ``pwm.c``. After
modification our Makefile should have a line like that one

``THIRD_PARTY_DATA = third-party/rboot/Makefile third-party/pwm/pwm.c``

Add external source to include path
===================================

If the new module has own header files that should be part of the
project you can add them to the ``EXTRA_INCDIR``. Here is an example
with the include paths that ``rboot`` brings

``EXTRA_INCDIR += third-party/rboot third-party/rboot/appcode``

Add external source to source code path
=======================================

If the new module brings also source code that needs to be compiled than
you can add it by modifying the ``MODULES`` variable. Here is an example
with ``esp-gdbstub``:

``MODULES       += third-party/esp-gdbstub``

Take a look at the Makefile to see examples of including the submodules
only when a switch is enabled (search for ENABLE_GDB).

Generating Patch Files
======================

It can happen that an external source code that is coming from another
git repository and is present as a submodule needs some changes before
it can start working for Sming. Those changes must be stored in a
separate patch file. The recommended way to generate a patch file is to
get the source code of the submodule, make changes to the file(s) in the
submodule until it is ready for use and finally generate a patch file
using the following commands:

::

   cd <Sming-root-folder>/third-party/<module-name>
   git diff --no-ext-diff > <Sming-root-folder>/third-party/.patches/<module-name>.patch
