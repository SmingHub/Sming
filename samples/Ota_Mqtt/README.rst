OTA over MQTT
=============

.. highlight:: bash

Introduction
------------

This example demonstrates how you can create an application that updates its firmware Over The Air (OTA) using the MQTT protocol.
MTQTT has less overhead compared to HTTP and can be used for faster delivery of application updates.

Versioning
----------
To simplify the OTA process and make it less error prone we are using the following versioning principles:
1) For version naming we use `semantic versioning <https://semver.org/>`_.
If our current version is 4.3.1 then 4 is the major, 3 is the minor and 1 is the patch version number.
2) Every application firmware knows its version.
3) An application with the same major and minor version should be compatible for update no matter what the patch number is.
If the new firmware is not compatible then a new minor or major version should be used.

Theory Of Operation
-------------------
1) On a period of time the application connects to check if there is a new version of the firmware.
In your application this period has to be carefully selected so that OTA updates occur when the device has
enough resources: memory, space on flash, power and time to complete such an update. Also there should be no critical task running at the moment.
Depending on the size of the new firmware and the speed of the connection an update can take 10 to 20 seconds.
2) The application connects via MQTT to a remote server and subscribes to a special topic. The topic is based on the
application id and its current version. If the current application version is 4.3.1 then the topic that will be used for OTA is "/a/test/u/4.3".
3) The application is waiting for new firmware. When the application is on battery than it makes sense to wait for a limited time and if there is no
message coming back to disconnect.

Firmware packaging
------------------
The firmware update must come as one MQTT message. The MQTT protocol allows messages with a maximum size of 268435455 bytes approx 260MB.
This should be perfectly enough for a device that has maximum 1MB available for an application ROM.
The message coming from MQTT contains:
- at the start one byte with the patch version of the firmware
- followed by the firmware data itself
For simplicity the patch version is just one byte. This limits us to 256 possible patch versions.
If needed the patch version can be encoded using `varint <https://developers.google.com/protocol-buffers/docs/encoding#varints>`_.
In this example there is no encoding, checksum, signature or encryption for the firmware data itself.

Security
--------
For additional security a standard SSL/TLS can be used
1) The communication should be secured using standard SSL.
2) To prove that the server is the correct one: The MQTT clients should pin the public key fingerprint on the server.
OR have a list of public key fingerprints that are allowed.
3) To prove that the clients are allowed to connect: Every MQTT client should also have a client certificate that is signed by the server.
