mqtt-protocol-c
===============

Zero-copy, interruptible MQTT protocol ``parser`` and ``serialiser``
written in C. Based initially on `Deoxxa’s
code <https://github.com/deoxxa/mqtt-protocol-c/>`__ and extended to
support full client and server parsing and serialization.

Overview
--------

mqtt-protocol-c is designed for use in resource-constrained
environments. To that end, it avoids making any memory allocations or
assumptions about the underlying system. It handles only the binary
parsing/serialising part of the MQTT protocol, leaving all the logic and
networking up to the user.

Examples
--------

Take a look at ``test_serialiser`` and ``test_parser`` in the
`bin/test.c <bin/test.c>`__ file.

License
-------

BSD-3 Clause license. You can read the full license from
`here <LICENSE.md>`__.

Copyright
---------

-  2018 - present Slavey Karadzhov slav@attachix.com
-  2014 - `Deoxxa
   Development <https://github.com/deoxxa/mqtt-protocol-c/>`__
