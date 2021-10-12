Using with Eclipse CDT
======================

.. highlight:: bash

The Eclipse CDT Project provides a fully functional C and C++ Integrated Development Environment based on the Eclipse platform.
Eclipse is a free (as in "free beer") and Open Source code editor for Windows, Linux and Mac.

For easier integration make sure you have both :envvar:`ESP_HOME` and
:envvar:`SMING_HOME` exported in your working environment.


Software involved
-----------------

-  `Eclipse CDT <https://www.eclipse.org/cdt/>`__

Installation
------------

-  Install Eclipse CDT using your operating system packaging tools.

Configuration
-------------

First you should import the Sming project.
This can be done by going to menu ``File`` -> ``Import`` and from there choosing 
``General`` -> ``Existing project into Workspace``. When asked for the location of the project
point Eclipse to the location of your SMING_HOME folder.


Once you have imported the Sming project you can import some of the samples.
Run again ``File`` -> ``Import`` and from there choose ``General`` -> ``Existing project into Workspace``.
Then select the ``Basic_Blink`` directory under ``samples``. Once the project is imported and the Eclipse CDT indexes
are build you will be able to program like a pro and have code completion and debug your code. 
For the latter see :sample:`LiveDebug`.

If your application is not containing `.project` and `.cproject` files inside its root folder then
you can create such files. First you need to go to the root folder of your project and then type the command below::

   make ide-eclipse
	
This will create the minimum required meta files for an Eclipse CDT project which you can import into your Workspace.