Library CI support
==================

Appveyor
--------

Steps to enable:

Add project to appveyor account
    ``Projects`` -> ``New Project`` and select from list

Set ``Custom Configuration``
    to ``https://raw.githubusercontent.com/SmingHub/Sming/develop/Tools/ci/library/appveyor.txt``

Set ``Project URL slug``
    If library exists as Sming submodule then set this to the same name so it overrides current Sming version.

    For example, a library called ``Sming-jerryscript`` will get a default slug of ``sming-jerryscript``,
    but this must be changed to ``jerryscript``.

Set sming fork/branch
    By default builds use the main Sming ``develop`` branch.
    To change this add ``SMING_REPO`` and/or ``SMING_BRANCH`` environment variables.


Makefile
--------

The provided default makefile builds all applications within the library's ``samples`` directory
for all architectures.

If a ``test`` directory is found then that too is built for all architectures, and executed for Host.
