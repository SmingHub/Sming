********************************
Using with MS Visual Studio Code
********************************


`Visual Studio Code <https://code.visualstudio.com/>`__ is free (as in
“free beer”, and they claim the code is Open Source) code editor for
Windows, Linux and Mac. While not as sophisticated in C/C++ support as
full featured Visual Studio, it already has some official support with
`C/C++ extension from
Microsoft <https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools>`__.
GNU Global replaces venerable ``ctags`` tool and help collecting symbols
for Intellisense engine.

For easier integration make sure you have both ``ESP_HOME`` and
``SMING_HOME`` exported in your working environment.
``${workspaceRoot}`` below is the directory with your project, this
notation is used also in VS Code config files. All environment variables
are available in configuration using this notation, eg. ``${HOME}`` etc.

Software involved
=================

-  `Visual Studio Code <https://code.visualstudio.com/>`__
-  `C/C++
   extension <https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools>`__
-  `RunOnSave
   extension <https://marketplace.visualstudio.com/items?itemName=emeraldwalk.RunOnSave>`__
-  `C++ Intellisense
   extension <https://marketplace.visualstudio.com/items?itemName=austin.code-gnu-global>`__
-  `GNU Global <https://www.gnu.org/software/global/>`__

Fire and forget way
===================

-  install VS Code, extensions and tools
-  download sming-qstart.py from
   https://github.com/zgoda/sming-qstart/blob/master/sming-qstart.py
-  run it from terminal, eg. ``python sming-qstart.py myproject``, see
   the `program
   documentation <https://github.com/zgoda/sming-qstart/blob/master/README.md>`__
   for detailed invocation options

Easy way
========

-  install VS Code, extensions and tools
-  clone project skeleton using https://github.com/zgoda/sming-skel.git
   as source
-  remove unwanted bits (at least ``.git`` directory)
-  update paths configuration in
   ``${workspaceRoot}/.vscode/c_cpp_properties.json`` - should already
   work out of the box with Linux providing you have ``ESP_HOME`` and
   ``SMING_HOME`` properly exported

Step by step
============

-  install VS Code, extensions and tools
-  update paths configuration in
   ``${workspaceRoot}/.vscode/c_cpp_properties.json`` so the list
   includes toolchain include path
   (``${ESP_HOME}/xtensa-lx106-elf/xtensa-lx106-elf/include``), Sming
   framework include paths (``${SMING_HOME}`` and
   ``${SMING_HOME}/system/include``) and possibly your project
   additional paths (eg. ``${workspaceRoot}/lib``), if you screw your
   configuration just close VS Code, delete this file and start from
   scratch
-  make sure ``path`` list in ``browse`` section contains the same
   entries as ``includePath`` list in root section
-  define RunOnSave task in your
   ``${workspaceRoot}/.vscode/settings.json`` (create file if does not
   exist) to regenerate GNU Global database on every save, eg:

.. code-block:: json

   {
     "emeraldwalk.runonsave": {
       "commands": [
         {
           "match": "\\.(c|cpp|h|hpp)$",
           "isAsync": true,
           "cmd": "gtags ${workspaceRoot}"
         }
       ]
     }
   }

-  create file ``${workspaceRoot}/.vscode/tasks.json`` and define tasks
   you want to run from command palette, eg minimal set:

.. code-block:: json

   {
     "version": "0.1.0",
     "command": "make",
     "isShellCommand": true,
     "showOutput": "always",
     "echoCommand": true,
     "suppressTaskName": true,
     "tasks": [
       {
         "taskName": "Build",
         "isBuildCommand": true
       },
       {
         "taskName": "Clean",
         "args": [
           "clean"
         ]
       },
       {
         "taskName": "Flash",
         "args": [
           "flash"
         ]
       }
     ]
   }

-  add tools and binary artifacts to ``.gitignore``, eg:

::

   out

   # development tools
   .vscode
   GTAGS
   GRTAGS
   GPATH
