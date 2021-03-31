Clang Tools
===========

`clang-format <https://releases.llvm.org/8.0.1/tools/clang/docs/ClangFormat.html>`__
is a tool that implements automatic source code formatting.
It can be used to automatically enforce the layout rules for Sming.

`clang-tidy <https://releases.llvm.org/8.0.1/tools/clang/tools/extra/docs/clang-tidy/index.html>`__
is a C++ “linter” tool to assist with diagnosing and fixing  typical programming errors
such as style violations, interface misuse, or bugs that can be deduced via static analysis.
It is provided as part of 

You can find details for the current release at https://releases.llvm.org/download.html.
Note that *clang-format* is part of the main **Clang** project, whilst *clang-tidy* can be
found in **clang-tools-extra**.


Installation
------------

In Ubuntu you should be able to install them using the following command::

   sudo apt-get install clang-format clang-tidy

See the the `download <http://releases.llvm.org/download.html>`__ page
of the Clang project for installation instructions for other operating
systems.

.. important::

   Different versions of clang-format can produce different results,
   despite using the same configuration file.

   We are using version 8.0.1 of clang-format and clang-tidy on our
   Continuous Integration (CI) System.
   
   You should install the same version on your development computer.



Configuration
-------------

Rules
~~~~~

The coding rules are described in the
`.clang-format <https://github.com/SmingHub/Sming/blob/develop/.clang-format>`__
file, located in the root directory of the framework.

You should not edit this file unless it is a discussed and agreed coding
style change.

IDE integration
~~~~~~~~~~~~~~~

There are multiple existing integrations for IDEs. You can find details
in the `ClangFormat documentation <https://clang.llvm.org/docs/ClangFormat.html>`__.

Eclipse IDE
^^^^^^^^^^^

For our Eclipse IDE, which is our preferred IDE, we recommend installing
the `CppStyle plugin <https://github.com/wangzw/CppStyle>`__. You can
configure your IDE to auto-format the code on “Save” using the
recommended coding style and/or format according to our coding style
rules using Ctrl-Shift-F (for formatting of whole file or selection of
lines). Read
`Configure CppStyle <https://github.com/wangzw/CppStyle#configure-cppstyle>`__
for details.

Usage
-----

Command Line
~~~~~~~~~~~~

Single File

   If you want to directly apply the coding standards from the command line
   you can run the following command::
   
      cd $SMING_HOME
      clang-format -style=file -i Core/<modified-file>
   
   Where ``Core/<modified-file>`` should be replaced with the path to
   the file that you have modified.

All files

   The following command will run again the coding standards formatter over
   all C, C++ and header files inside the ``Sming/Core``, ``samples`` and 
   other key directories::
   
      cd $SMING_HOME
      make cs
   
   The command needs time to finish. So be patient. It will go over all
   files and will try to fix any coding style issues.
   
   If you wish to apply coding style to your own project, add an empty ``.cs`` marker file
   to any directory containing source code or header files. All source/header files
   in that directory and any sub-directories will be formatted when you run::
   
      make cs
   
   from your project directory.
   
Eclipse
~~~~~~~

If you have installed CppStyle as described above you can
configure Eclipse to auto-format your files on *Save*.

Alternatively, you can manually apply the coding style rules by selecting the source code of a
C, C++ or header file or a selection in it and run the ``Format`` command
(usually Ctrl-Shift-F).

