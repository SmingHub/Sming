Basic IFS
=========

.. highlight:: bash

Simple Webserver demonstration using IFS.

View the filesystem using a web browser.

To see the data in a different format, add ``?format=XX``. Current supported formats are ``json``, ``text`` and ``html``.

Building
--------

By default, data is stored in a read-only FWFS (Firmware Filesytem) partition.

This sample also demonstrates how to store the data in a :cpp:class:`FlashString` object::

    make config-clean
    make ENABLE_FLASHSTRING_IMAGE=1

Because the data is linked into the program image this is only suitable for small filesystem images.
This could be used to store default recovery data, especially with OTA updates because each program
image is self-contained.
