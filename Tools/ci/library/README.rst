Library CI support
==================

.. highlight:: bash

Appveyor may be configured to test a Sming library separately.


Appveyor
--------

Steps to enable:

Add project to appveyor account
    ``Projects`` -> ``New Project`` and select from list

Set ``Custom Configuration``
    to ``https://raw.githubusercontent.com/SmingHub/Sming/develop/Tools/ci/library/appveyor.txt``.

Set ``Project URL slug``
    If the library under test already exists in the Sming framework then the test directory
    MUST have the same name to ensure it gets picked up.

    For example, testing the ``Sming-jerryscript`` library requires this value to be set to ``jerryscript``
    to match the Sming library name.
    Build logs should then report a warning ``Multiple matches found for Component 'jerryscript'.

Set sming fork/branch
    By default builds use the main Sming ``develop`` branch.
    To change this add ``SMING_REPO`` and/or ``SMING_BRANCH`` environment variables.


Makefile
--------

The provided default makefile builds all applications within the library's ``samples`` directory.
If a ``test`` directory is found then that too is built, and executed for Host.


Issues
------

If you get error ``image not supported by cloud`` then goto Appveyor -> BYOC -> Images and delete the offending entries.
