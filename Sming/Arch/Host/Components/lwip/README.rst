LWIP
====

Introduction
------------

Uses LWIP version 2 to enable networking for Sming running on a Windows or Linux Host system.

.. note::

   Network support is enabled by default. If you don't need it, use the ``--nonet`` command-line option.

Build Variables
---------------

.. envvar:: ENABLE_LWIPDEBUG

   0 (default)
      Standard build
   1
      Enable debugging output


.. envvar:: ENABLE_CUSTOM_LWIP

   2 (default)
   
   Setting this to any other value will cause a build error.

Linux
-----

Support is provided via TAP network interface (a virtual network layer
operating at the ethernet frame level). A TAP interface must be created
first, and requires root priviledge::

   sudo ip tuntap add dev tap0 mode tap user `whoami`
   sudo ip a a dev tap0 192.168.13.1/24
   sudo ifconfig tap0 up

This creates the ``tap0`` interface. The emulator will automatically
select the first ``tap`` interface found. To override this, use the
``--ifname`` option. An IP address will be assigned, but can be changed
using the ``--ipaddr`` option.

If your application needs to access the internet, additional setup is
required::

   sudo sysctl net.ipv4.ip_forward=1
   sudo sysctl net.ipv6.conf.default.forwarding=1
   sudo sysctl net.ipv6.conf.all.forwarding=1

   export INTERNET_IF=wlan0 # <!--- Make sure to replace wlan0 with the network interface connected to Internet

   sudo iptables -t nat -A POSTROUTING -o $INTERNET_IF -j MASQUERADE
   sudo iptables -A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
   sudo iptables -A FORWARD -i tap0 -o $INTERNET_IF -j ACCEPT

Windows
-------

Requires `NPCAP <https://nmap.org/npcap/>`__ library to be installed.
Provided with current (3.0.2) version of
`Wireshark <https://www.wireshark.org/download.html>`__.

By default, the first valid network adapter will be used, with address
assigned via DHCP.

If the adapter is wrong, get a list thus:

.. code-block:: batch

   out\Host\Windows\debug\firmware\app --ifname=?

or

.. code-block:: batch

   make run HOST_NETWORK_OPTIONS=--ifname=?

produces a listing:

.. code-block:: text

   Available adapters:
   - 0: {ACC6BFB2-A15B-4CF8-B93A-8D97644D0AAC} - Oracle
           192.168.56.1 / 255.255.255.0
   - 1: {A12D4DD0-0EA8-435D-985E-A1F96F781EF0} - NdisWan Adapter
   - 2: {3D66A354-39DD-4C6A-B9C4-14EE223FC3D1} - MS NDIS 6.0 LoopBack Driver
           0.0.0.0 / 255.0.0.0
   - 3: {BC53D919-339E-4D70-8573-9D7A8AE303C7} - NdisWan Adapter
   - 4: {3CFD43EA-9CC7-44A7-83D4-EB04DD029FE7} - NdisWan Adapter
   - 5: {530640FF-A9C3-436B-9EA2-65102C788119} - Realtek PCIe GBE Family Controller
           192.168.1.70 / 255.255.255.0
   - 6: {0F649280-BAC2-4515-9CE3-F7DFBB6A1BF8} - Kaspersky Security Data Escort Adapter
           10.102.37.150 / 255.255.255.252

Then use the appropriate number (or GUID), with the gateway IP address -
an address will be assigned via DHCP:

.. code-block:: batch

   make run HOST_NETWORK_OPTIONS="--ifname=5 --gateway=192.168.1.254"

You can find gateway addresses using the ``ipconfig`` command.

If you want to use a specific IP address, the appropriate adapter will
be selected but you still need to specify the gateway address:

.. code-block:: batch

   make run HOST_NETWORK_OPTIONS="--ipaddr=192.168.1.10 --gateway=192.168.1.254"
