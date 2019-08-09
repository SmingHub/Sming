Echo SSL
========

This example talks to an SSL-enabled TCP server. If you do not have such you can use ncat (from the nmap package) and run something like this:

.. code-block:: bash

   ncat -vvv  -l 0.0.0.0 4444 --ssl --ssl-key ~/Private/x.key --ssl-cert ~/Private/x.cert


See :sample:`Basic_Ssl` for information on compiling and configuring SSL.
