Documentation System
====================

.. highlight:: rst

Read the Docs and Sphinx
------------------------

Online documentation is managed via
`Read the Docs <https://docs.readthedocs.io/en/stable/index.html>`_,
which uses `Sphinx <https://www.sphinx-doc.org>`_ as the documentation
build system.

This page describes specific details for the Sming documentation build system.

reStructuredText
----------------

These have a ``.rst`` file extension. Some references you should have a read through:

* `reStructuredText Primer <http://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html>`_
* `An introduction to Sphinx and Read the Docs for technical writers <http://ericholscher.com/blog/2016/jul/1/sphinx-and-rtd-for-writers/>`_

Markdown ``.md`` files are supported (except for samples) but please ensure there is only one
main heading (*\*HEADING*) at the top containing the title of the Library or Component or it
may be incorrectly rendered in the table of contents. If you run into problems trying to get
your markdown to display correctly, then it's probably time to move to reStructuredText!

If you need to convert existing documentation into reStructuredText take
a look at `pandoc <https://pandoc.org/>`_. Note that their
`online tool <https://pandoc.org/try/>`_ doesn't handle long files.

Source file location
--------------------

Sphinx requires all document files (documents, images, etc.) to be
located in one place, namely inside the ``docs/source`` directory.
During the build process, documentation files from the Component, Library
and Sample directories are copied:

.. code-block:: text

   README.md
   *.rst
   *.svg
   *.png
   *.jpg

This is also performed for any submodules declared by the COMPONENT_SUBMODULES
variable in a component.mk file.

This should be sufficient for most cases. However, if additional
documentation is referred to, such as in submodules belonging to a
Component, then these must be listed in the ``component.mk`` file like
this:

.. code-block:: make

   COMPONENT_DOCFILES := submodule/docs/api-guide.md submodule/api.txt

See :doc:`/_inc/Sming/building` for more information about component.mk files.

README files
------------

All Components, Libraries and Samples must include a ``README.rst`` or ``README.md`` file as follows:

  - **Main Heading**: This will be used as the title in the online documentation, so keep it brief but informative.
    For example, ``BME280 Barometric Pressure Sensor``.
  - **Purpose**: What is the purpose of the code?
  - **References**: Is this based on or does it use existing code? Please include details.
  - **Datasheets**: If appropriate, please include links to manufacturer's or external development websites.
    Note that any submodules or dependencies are automatically documented: see :doc:`/_inc/Sming/building` for details,
    specifically :envvar:`COMPONENT_SUBMODULES` and :envvar:`COMPONENT_DEPENDS`.

You should also try to include any other information which could be useful information for a new developer.
The purpose of samples projects is to demonstrate specific features or libraries, so please ensure this is adequately described.

Optionally, you may also like to include a screenshot or other diagrams or illustrations.
Please use ``.png``, ``.jpg`` or ``.svg`` files.

.. attention::
   The README filename is case-sensitive

.. attention::
   Please ensure there is only one top-level heading or the documentation contents will not display correctly.

You should use the available annotations to make browsing the documentation easier. Using the
`Sphinx roles <https://www.sphinx-doc.org/en/master/usage/restructuredtext/roles.html>`
will insert hyper links to the corresponding definitions.
Additional roles are provided specifically for the Sming documentation - see `link_roles`_ below.

Code blocks
-----------

There are multiple ways to show syntax-higlighted literal code blocks in
Sphinx. See
`Showing code examples <https://www.sphinx-doc.org/en/master/usage/restructuredtext/directives.html?highlight=code-block#showing-code-examples>`__
for full details.

Use the ``code-block`` directive like so::

   .. code-block:: c++
   
      for(int i = 0; i < 100; ++i) {
         goto hell;
      }

The language for highlighting is indicated. You can find a full list at
`pygments.org <http://pygments.org/docs/lexers/>`__, however for
consistency it is suggested that you use one of these:

.. code-block:: text

   text     Doesn't highlight anything
   c++      C++ code examples
   bash     Linux shell code
   batch    Windows batch file or commands
   make     Makefile
   rst      reStructuredText

You can set a default like this::

   .. highlight:: c++
   
which will apply to any subsequent use of::

   .. code:block::

or, the short-hand version::

   ::

API Documentation
-----------------

Function, structure, class and type information is extracted from
comments in the source code (see :doc:`documenting-the-api`). This is
parsed using `Doxygen <http://www.doxygen.nl/index.html>`_ into XML,
which is then made available using the
`Breathe <https://breathe.readthedocs.io/en/latest/>`_ sphinx
extension. You can then pull in definitions like this::

   .. doxygenclass:: String
      :members:

If you wish to refer to a type within documentation, you can add a link
to the definition like this::

   The :cpp:class:`String` class is really useful.

This is handled using
`cpp inline expressions <https://www.sphinx-doc.org/en/master/usage/restructuredtext/domains.html#inline-expressions-and-types>`_.


See :component-esp8266:`gdbstub` for a more complex example.
At the bottom of the file we pull in the documentation for all the
#defined configuration using::

   .. doxygenfile:: gdbstub-cfg.h

We can then refer to a macro like this::

   Don't wait on startup by setting :c:macro:`GDBSTUB_BREAK_ON_INIT` =0

In many cases including a file like this is not the best approach,
perhaps using a group::

   .. dogygengroup:: wstring

Or individual classes. Some experimentation may be necessary but there
are plenty of examples within the main documentation to guide you.

You can use the following build variables within your Component's
component.mk file to direct doxygen parsing:


.. envvar:: COMPONENT_DOXYGEN_INPUT

   Specifies directories or files to be parsed by Doxygen.
   All paths are relative to the Component directory.

   If you need to specify an absolute path, append directly
   to DOXYGEN_INPUT instead.


.. envvar:: COMPONENT_DOXYGEN_INCLUDE

   Add any directories or files which should be pre-processed but
   not included in the output.

   If you need to specify an absolute path, append directly
   to DOXYGEN_INCLUDE_PATH instead.


.. envvar:: COMPONENT_DOXYGEN_PREDEFINED

   Specify any necessary pre-processor definitions.
   An example where this is required is for function attributes #defines
   which would otherwise be incorrectly interpreted as variable names
   and cause parsing errors::

      CUSTOM_ATTR void myFunc();
      ^^^

   So we can do this::

      COMPONENT_DOXYGEN_PREDEFINED := \
         CUSTOM_ATTR=


Build (environment) variables
-----------------------------

These are defined in the README for the corresponding Component using::

   :envvar::`COM_SPEED`
   Determines default serial port speed

You can refer to them like this::

   Change baud rate using the :envvar:`COM_SPEED` variable.


.. _link_roles:

Link Roles
----------

The documentation build system provides some custom roles for creating links.

Components
~~~~~~~~~~

Inserting a link to a Component page, using the title of that page by default::

   See :library:`Spiffs` for details of the flash filing system.

   We use :component-esp8266:`axtls-8266` for SSL support.

   The host has a special :component-host:`UART Driver <driver>`.

The last example shows how to change the hyperlink text. It defaults to
the README description.

Libraries
~~~~~~~~~

As for Components, refer to libraries like this::

   Use the :library:`Adafruit_ST7735` library to do some fancy display stuff.


Sample applications
~~~~~~~~~~~~~~~~~~~

To refer to a sample application README::

   See the :sample:`Basic_Blink` sample for a simple introduction to Sming.

Samples may be located in the main Sming :source:`samples`, but a growing number
are located in a **samples** sub-directory of the associated Component or library.

Another example::

   Where is the :sample:`generic` sample?

There are currently three libraries with a sample called 'generic'.
Documents within the appropriate library will be matched correctly.

The given target 'generic' is matched as follows:

- Is the document contained in a Component or library?
  - Yes: Look in the Component's **samples** sub-directory, if there is one
- No match? Then check the main sming **samples** directory.
- Still no match? Pick the first match from any other Component.
- No match found? Text will appear unchanged in the output document.

To refer to a sample in a specific Component, do this::

   I'm looking for the :sample:`CS5460/generic` sample.
   Where is :sample:`Generic CS5460 sample <CS5460/generic>` sample?

Within main samples directory::

   Is there a main :sample:`/generic` sample? Actually, no.
   But there is a :sample:`/CanBus` sample!


Source code
~~~~~~~~~~~

Use within the framework
   Use the file or directory path relative to the root directory. For example::

      See :source:`Sming/Core/DateTime.h`

   will create a hyperlink to the source file in the Sming repository on github.

Use within a Component or library
   The source link will be interpreted relative to the Component root directory.
   You can find an example of this usage in the :library:`UPnP` README::

      :source:`samples/Basic_UPnP/include/Wemo.h#L59-L91`

   If you wish to refer to a source file elsewhere in Sming, prefix with '/'::

      See :source:`/tests/HostTests/modules/Timers.cpp` for an example.


Issues and Pull Requests
~~~~~~~~~~~~~~~~~~~~~~~~

If you want to refer to discussions on github, insert links like this::

   See :pull-request:`787`

   See :issue:`1764`


Eclipse
-------

You can find a good plugin editor for Eclipse by searching the
marketplace for ``rest editor``. For example,
http://resteditor.sourceforge.net/. A useful feature is dealing with
heading underscores, just type this::

   My Heading
   ==

Then when you save the file it gets formatted like this::

   My Heading
   ==========

Tables, unfortunately, do take a bit of manual formatting to get right.


Sphinx Extensions
-----------------

The documentation system is easily extended to support new features.
This section summarises the extensions included.

`m2r2 <https://github.com/crossnox/m2r2>`__
      Provides support for markdown content.


`breathe <https://github.com/michaeljones/breathe>`__
   To support Doxygen integration. See `API Documentation <#api-documentation>`_.


`link-roles <link-roles.py>`_
   A custom extension implemented in *link-roles.py*. See `Link Roles <#link-roles>`_.


`sphinxcontrib.wavedrom <https://github.com/bavovanachte/sphinx-wavedrom>`__
   For implementing timing and other waveform diagrams within documents.
   See :library:`Servo` for an example.

`sphinxcontrib.seqdiag <https://github.com/blockdiag/sphinxcontrib-seqdiag>`__
   For embedding sequence diagrams, such as in the :doc:`/information/tasks` page.
