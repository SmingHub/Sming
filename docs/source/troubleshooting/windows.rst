***********************
Troubleshooting Windows
***********************

If something goes wrong - don’t worry, community will be able to help
you out. Don’t forget to check
`User questions <https://github.com/SmingHub/Sming/issues?q=label%3Aquestion+is%3Aall>`__
before posting a github issues. Maybe someone else had a similar issue!

If nothing found, please make sure to provide all required information
when posting issues. Here’s the minimum that you will need to get:
Start ``cmd.exe`` and provide output of the following commands:

::

   echo %PATH%
   echo %SMING_HOME%
   echo %ESP_HOME%
   dir %SMING_HOME%
   dir %ESP_HOME%
   where make

Common issues & solutions
=========================

-  ``SMING_HOME`` should be set to ``c:\tools\sming\Sming``, with
   ``\`` as a path separator, and NO backslash ``\`` at the end.
-  ``ESP_HOME`` should be set to ``c:\Espressif``, using ``\`` as a
   path separator, and NO backslash ``\`` at the end.
-  MinGW paths should be at the start of PATH environment variable
   (before other items).
-  If you update your sming-core source don’t forget to do
   ``cd c:\tools\sming\Sming && make clean && make``

