Sming Documentation
===================

.. highlight:: bash

This directory contains the build system for Sming's documentation.
It is intended to be read online via `Read The Docs <https://sming.readthedocs.io>`__.

The ``source`` directory contains some top-level files however most of the
information is obtained from the various README files associated with the source code.

Setup
-----

Linux::

	../Tools/install.sh doc

Windows::

	..\Tools\install.cmd doc


Building
--------

Build the documentation like this::

	make html

This will:

- Pull in and patch every submodule
- Generate doxygen API information
- Build documentation in HTML format

If you make changes to any source documentation files these will be
picked up automatically when ``make html`` is next run.

If you make any changes to source code comments, you'll need to re-build
the doxygen information first::

   make api -B
   make html
