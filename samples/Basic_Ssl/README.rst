Basic SSL
=========

Compilation
-----------

In Sming the SSL support is not enabled by default.

In order to enable it you should compile your project with the
:envvar:`ENABLE_SSL` =1 directive. This can be done using the following command:

.. code:: bash

   make ENABLE_SSL=1

Now you can flash your application to your ESP8266 device.

Debug Information
-----------------

If you want to see more debug information during compile type you should
add the directive :envvar:`SSL_DEBUG` =1, like this:

.. code:: bash

   make ENABLE_SSL=1 SSL_DEBUG=1

Slow SSL negotiation
--------------------

The initial SSL negotiation is CPU intensive. By default SmingFramework
switches the CPU frequency from 80 to 160 MHz. After the negotiation the
CPU is switched back to 80 MHz.

If your device is running on battery this can drain the battery much
faster. If you do not want the switch from 80 to 160 MHz to happen then
make sure to recompile SmingFramework with :c:macro:`SSL_SLOW_CONNECT` directive:

.. code:: bash

   make USER_CFLAGS="SSL_SLOW_CONNECT=1"
