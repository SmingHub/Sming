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
You need to open in terminal the location of the sample of your interest and type::

    cd /path/to/sample/
    make ide-eclipse

This will create the minimum required meta files for an Eclipse CDT project which you can import into your Workspace.
``/path/to/sample/`` should be replaced with the actual location of the sample. 
For the ``Basic_Blink`` sample we can use the following commands::

	cd $SMING_HOME/../samples/Basic_Blink # Replace $SMING_HOME with %SMING_HOME% if you use Windows
	make ide-eclipse

After this you go back to your Eclipse and run again ``File`` -> ``Import`` and from there choose ``General`` -> ``Existing project into Workspace``.
Then select the ``Basic_Blink`` directory under ``samples``. Once the project is imported and the Eclipse CDT indexes
are build you will be able to program like a pro and have code completion and debug your code. 
For the latter see :sample:`LiveDebug`.

You can import also your own Sming-based applications in Eclipse. 
If your application is not containing `.project` and `.cproject` files inside its root folder then
you can create such files by going to the root folder of your application and then typing the command below::

   make ide-eclipse
	
