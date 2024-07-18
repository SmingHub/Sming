HostTests
=========

Modular tests which must build and run on all architectures.

DateTime
--------

Test data in ``include/DateTimeData.h`` is generated using python script ``tools/datetime-test.py``.

If this file is changed, re-generate by running::

    tools/datetime-test.py include/DateTimeData.h
