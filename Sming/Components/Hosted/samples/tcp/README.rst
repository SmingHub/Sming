Hosted Server Application
=========================

TBD...


Testing
-------

You can compile the Hosted App to run also under the Host system. This can be done with the following command::

    cd $SMING_HOME/Components/Hosted/app
    make run SMING_ARCH=Host ENABLE_GDB=1

Once the HostedServer is up and running you can send protobuffer encoded commands to it. A sample test client can be
run with the following command::

    nc 192.168.13.10 4031 < test/data.pb



