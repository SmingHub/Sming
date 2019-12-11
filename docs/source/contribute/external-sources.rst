Adding External Sources
=======================

.. highlight:: bash

Introduction
------------

In Sming we have source code from other repositories such as
`rboot <https://github.com/raburton/rboot>`__,
`spiffs <https://github.com/pellepl/spiffs>`__, etc.

Having local copies of those modules brings some disadvantages with it:

1. We cannot easily update the local copy from the original source code.
2. We cannot easily send improvements to those projects once we make
   local changes in our local copy.

Sming uses `GIT submodules <https://git-scm.com/book/en/v2/Git-Tools-Submodules>`__
which allows the build system to fetch source code from an external repository on demand.

If modifications are required then the submodule can be `patched <#applying-patches>`_.

This simplifies the process of integrating changes from those third-party projects.

Where to place external code
----------------------------

Submodules may be a :ref:`component` by itself (such as :component:`FlashString`),
or contained within a Component (e.g. :component:`rboot`).

The location must be chosen carefully:

Code required within the core Sming framework

   If the Component supports multiple architectures, place it in ``Sming/Components``.
   Otherwise, use the appropriate ``Sming/Arch/*/Components`` directory.

Code for general use

   Create a new Library in ``Sming/Libraries``

Please consult :doc:`/_inc/Sming/building` for further details about how Components are constructed.


Copying Source Code
-------------------

If the source code does not have a publicly accessible GIT repository
then the source code needs to be copied locally.

In order to track changes more easily, the initial commit should be an exact
copy of the original.

Please either comment the code or add notes to the documentation to detail
any required changes for compatibility.


Add submodules
--------------

As an example, this is how the `new PWM` submodule was added to the :component-esp8266:`driver` Component:

1. Add the submodule using GIT::

      cd <Sming-root-folder>/
      git submodule add \
         https://github.com/StefanBruens/ESP8266_new_pwm.git \
         Sming/Arch/Esp8266/Components/driver/new-pwm \
         --name ESP8266.new-pwm

This adds an entry to the end of the ``.gitmodules`` file::

   [submodule "ESP8266.new-pwm"]
      path = Sming/Arch/Esp8266/Components/driver/new-pwm
      url = https://github.com/StefanBruens/ESP8266_new_pwm.git

For naming submodules, please follow the convention used for the other entries in
``.gitmodules``, which is determined by the local path::

-  ``Sming/Components``: just use the name  of the submodule
-  ``Sming/Arch/ARCH/Components``: Use ``ARCH.name``
-  ``Sming/Libraries``: Use ``Libraries.name``

2. Open ``.gitmodules`` in a text editor and:

a. Move the entry to a more suitable location in the file, i.e. at the end of the
   section listing all the ESP8266-specific submodules
b. Add the line ``ignore = dirty``


Applying Patches
----------------

If a submodule requires changes to work with Sming, this can be handled using patches.

This is generally done by pulling in the original submodule, making the required changes
and then running a ``diff`` to create a patch file, like this::

::

   cd <Sming-root-folder>/third-party/<module-name>
   git diff --no-ext-diff > <module-name>.patch

If using a GUI such as GIT Extensions then this can be done using a right-click.

See :ref:`git_submodules` for further details about how patches are used and where they should be placed.


Using submodules
----------------

If the submodule is added as a Component in its own right, no further action is required.
Applications can use it by adding the name to their COMPONENT_DEPENDS or ARDUINO_LIBARIES
entries in component.mk as appropriate.

Submodules contained within a Component must be declared by adding them to the
COMPONENT_SUBMODULES entry in component.mk.

