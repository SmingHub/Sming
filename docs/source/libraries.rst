Libraries
=========

Sming comes with a number of ported libraries that cover many areas of
embedded programming but for sure not all.

To reduce the size of the framework, some libraries are not included directly
and must be retrieved if you wish to access any sample applications there.

For example, the :sample:`Basic_UPnP` sample application is contained in the
:library:`UPnP` library, which can be retrieved like this::

   cd $SMING_HOME
   make fetch UPnP

You should then get *UPnP: found in 'Libraries/UPnP'*, so now we can build the sample::

   cd Libraries/UPnP/samples/Basic_UPnP
   make

.. note::

   If your project references any libraries they will automatically be
   pulled in during the build.

These are all the libraries included with Sming:

.. toctree::
   :glob:
   :maxdepth: 1
   :titlesonly:

   /_inc/Sming/Libraries/*/index

