Device_Scanner
==============

.. highlight:: bash

Utility to package and deploy firmware files for upgrade over MQTT
See :library:`OtaUpgradeMqtt` for details.


Packaging
---------

To scan your local network, do this::

   make pack HOST_PARAMETERS=scan


This will start by scanning for all root devices ``:upnp:rootdevice``.
If you want to use a specific search type, just add it to the command line::

   make run HOST_PARAMETERS='scan urn:schemas-upnp-org:service:ContentDirectory:1'

This will take some time to complete. If there are any failures, you can retry
by running the command again: any descriptions which have already been fetched will
be skipped.

You find output in the following directories:

out/upnp/devices
   Contains a hierarchical map of all found devices on your local UPnP network.


The schema directories are populated with modified versions of the description files.
These reflect the C++ class structures which we will later create:

out/upnp/schema/services
   All services are extracted into a directory by ``{domain}``. Each service has a separate .xml file.

   Service schema are re-usable and new ones should be added to the UPnP library so they are available for
   everyone to use.


out/upnp/schema/devices
   All devices (both root and embedded) are extracted to a separate directory ``{manufacturer}/{friendlyName}``
   with a single .xml file for each device.

   Device schema are specific to each device implementation so will generally be kept in a separate library
   or application.


Processing existing descriptions
--------------------------------

You can also process device descriptions files pulled in via other means. For example:

   make run HOST_PARAMETERS='parse config/panasonic/viera dmr/ddd.xml dms/ddd.xml nrc/ddd.xml'

The first parameter ``parse`` is the command.
The second parameter ``config/panasonic/viera`` gives the root directory for the device.
The remaining parameters are the relative locations from this directory of a device description file.
References to service files are pulled in: if they are missing, this may fail.

