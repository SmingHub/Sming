CS5460 generic sample
=====================

.. highlight:: bash

The sample prints the measured RMS voltage each second (with CS5460 voltage and current filters enabled).

The default MISO pin (12) can be changed through the envvar:`MISO` variable. For example::

   make MISO=13

These variables can be listed with::

   make list-config
