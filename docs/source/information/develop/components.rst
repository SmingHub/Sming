Component Building Guide
========================

.. highlight:: bash

Introduction
------------

This section will take you through creating a Sming Component from start to finish,
based on a real example from :pull-request:`1992`.

The aim here is to make an existing library, https://github.com/nomis/ModbusMaster,
available for applications to use within the framework.

Various options and settings are only mentioned here, but are linked through to
:doc:`building` where you can find full details of what they are and how they work.

You will need to be familiar with GIT and the use of submodules.

Structure
---------

You should carefully consider how to incorporate third-party code. The choices are:

a. Add an existing third-party library directly as a Component, patching if required;
b. Add the a submodule as a sub-directory within a Component;
c. Copy code from an existing  library (honouring any license agreement conditions)
   and modify/rewrite as necessary;
d. Start from scratch. If drawing on ideas from other implementations those should be attributed.

We could use (a), but that is appropriate only for very simple libraries, or those written
for Sming but stored in an external repository (e.g. :component:`FlashString`).
You can find more details on this process in :doc:`external-sources`.

.. note::

   Bear in mind that the asynchronous architecture of Sming can be at odds with code written for Arduino.
   In this example, the main ``ModbusMasterTransaction`` code incorporates a loop which
   effectively halts program execution until a timeout occurs or a response is received.
   Fixing that could require extensive patching, so option (c) or (d) may be more appropriate.

For now we are using option (b), so we'll start by creating the Component directory::

   cd $SMING_HOME/Libraries
   mkdir ModbusMaster
   cd ModbusMaster

.. note::

   Do not use ``$SMING_HOME/Components``. This is reserved for framework libraries.

Now add the submodule::

   git submodule add --name Libraries.ModbusMaster https://github.com/nomis/ModbusMaster

This does three things:

1. Clones the external repository to the ``ModbusMaster`` sub-directory;
2. Adds an entry to the ``.gitmodules`` file with the URL, local path and specified name; 
3. Stages both changes, with a link indicating which commit should be used.

.. note::

   The dotted name format is a Sming convention.

Before committing, we need to edit the ``.gitmodules`` file. Open in a text editor and
follow the instructions therein.

You should now re-stage and commit the changes before proceeding.


Patching
--------

Where minor changes are required to third-party code then these can be applied automatically
as patches. See :doc:`external-sources` and :ref:`git_submodules` for details.

For ModbusMaster, we want to make a slight change to how the timeout is configured,
so this is provided in a file called ``ModbusMaster.patch``.


Supported architectures
-----------------------

Unless there are specific reasons not to do so, Components should work on all
supported architectures. In particular, it should build and run under the
:doc:`Host Emulator </arch/host/index>`.

In order to do this, you should remove any low-level code from the library by:

-  Using Sming support classes or drivers (see :component-esp8266:`driver`); or
-  Placing low-level code into separate code modules or header files.

If a Component is intended only to work with specific hardware then ensure
appropriate checks are incorporated so that building fails gracefully on un-supported
architectures. You can do this in the component.mk file:

.. code-block:: make

   ifneq ($(SMING_ARCH),Esp8266)
   $(error MyComponent currently only supports the ESP8266 architecture)
   endif


Component configuration
-----------------------

For very simple Components the default settings are adequate:

-  Source code must be in the base directory or a sub-directory called ``src``
-  Public header files must be in a sub-directory called ``include``

The source files will be compiled into a library, in this case ``clib-ModbusMaster.a``.

To change the defaults, provide a ``component.mk`` file and set entries as required:

Submodules
   We need to tell Sming about the submodules::

      COMPONENT_SUBMODULES := ModbusMaster

Source code
   Put the source code into a separate directory (or directories) and add those to
   :envvar:`COMPONENT_SRCDIRS`.

   You can also use :envvar:`COMPONENT_SRCFILES`.

   For ModbusMaster, all the source code is in the submodule so we set this
   to ``COMPONENT_SRCDIRS := ModbusMaster/src``.

Header files
   Keep public and private ``.h`` or ``.hpp`` files in separate directories.

   Add the public ones to :envvar:`COMPONENT_INCDIRS`.

   Any private headers can be set in :envvar:`EXTRA_INCDIR`.
   
   For ModbusMaster, we set this to ``COMPONENT_INCDIRS := ModbusMaster/src``


Configuration options
   ModbusMaster provides the :envvar:`MB_RESPONSE_TIMEOUT` variable.

   This is mapped onto a ``#define`` value with the same name using :envvar:`COMPONENT_CXXFLAGS`.

   Note: Don't confuse this with :envvar:`COMPONENT_CFLAGS` which is only used when
   building ``.c`` source files.

   If the value changes we want Sming to rebuild both the library **and** any code which uses it,
   so we assign it to the :envvar:`COMPONENT_VARS` variable list.
   Users can check the value by running ``make list-config``.

Dependencies
   If your library depends on other libraries to build, these must be declared in the
   ``component.mk`` file by setting :envvar:`COMPONENT_DEPENDS` variable.

   ModbusMaster doesn't have any so this entry is not required.



Documentation
-------------

A Component **MUST** have a ``README.md`` (markdown) or ``README.rst`` (reStructuredText) file
with a level 1 heading and brief introduction at an absolute minimum.

.. note::

   You may not be familiar with ``.rst`` files but they are a considerably improvement on markdown
   and well worth investing a little time to learn.
   
   See :doc:`/contribute/documentation` for further details.

