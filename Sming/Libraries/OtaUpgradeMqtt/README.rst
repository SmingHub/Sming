OTA Firmware Upgrade via MQTT
=============================

.. highlight:: bash

Introduction
------------

This library allows Sming applications to upgrade their firmware Over-The-Air (OTA) using the MQTT protocol.
MTQTT has less overhead compared to HTTP and can be used for faster delivery of application updates.

Versioning Principles
---------------------
To simplify the OTA process we strongly recommend the following versioning principles for your application:

1. Use `semantic versioning <https://semver.org/>`_.
   If your current application version is 4.3.1 then 4 is the major, 3 is the minor and 1 is the patch version number.

2. Every application firmware knows its version.

3. An application with the same major and minor version should be compatible for update no matter what the patch number is.
   If the new firmware is not compatible then a new minor or major version should be used.

Theory Of Operation
-------------------
1. On a period of time the application connects to check if there is a new version of the firmware.
   In your application this period has to be carefully selected so that OTA updates occur when the device has
   enough resources: memory, space on flash, power and time to complete such an update. Also there should be no critical task running at the moment.
   Depending on the size of the new firmware and the speed of the connection an update can take 10 to 20 seconds.

2. The application connects via MQTT to a remote server and subscribes to a special topic. The topic is based on the
   application id and its current version. If the current application version is 4.3.1 then the topic that will be used for OTA is ``/a/test/u/4.3``.

3. If there is a need to support both stable and unstable/nightly builds then the topic name can have `s` or `u` suffix. For example
   all stable versions should be published and downloaded from the topic ``/a/test/u/4.3/s``. For the unstable ones we can use the topic ``/a/test/u/4.3/u``.
   If an application is interested in both then it can subscribe using the following pattern ``/a/test/u/4.3/+``.

4. The application is waiting for new firmware. When the application is on battery than it makes sense to wait for a limited time and if there is no
   message coming back to disconnect.

Firmware packaging
------------------
The firmware update must come as one MQTT message. The MQTT protocol allows messages with a maximum size of 268435455 bytes approx 260MB.
This should be perfectly enough for a device that has maximum 1MB available for an application ROM.

One MQTT message contains:

- patch version of the firmware
- followed by the firmware data itself

Based on the :envvar:`ENABLE_OTA_VARINT_VERSION` the patch version can be encoded either using one byte or a `varint <https://developers.google.com/protocol-buffers/docs/encoding#varints>`_.
Based on :envvar:`ENABLE_OTA_ADVANCED` the firmware data can be either without any encoding or be signed and encrypted.

Security
--------
For additional security a standard SSL/TLS can be used

1. The communication should be secured using standard SSL.

2. To prove that the server is the correct one: The MQTT clients should pin the public key fingerprint on the server.
   OR have a list of public key fingerprints that are allowed.

3. To prove that the clients are allowed to connect: Every MQTT client should also have a client certificate that is signed by the server.

Configuration
-------------

.. envvar:: ENABLE_OTA_VARINT_VERSION

   Default: 1 (enabled)

   If set to 1 the OTA upgrade mechanism and application will use a `varint <https://developers.google.com/protocol-buffers/docs/encoding#varints>`_
   encoding for the patch version. Thus allowing unlimited number of patch versions. Useful for enumerating unstable/nightly releases.
   A bit more difficult to read and write but allows for unlimited versions.

   If set to 0 the OTA upgrade mechanism and application will use one byte for the patch version which will limit it to 256 possible patch versions.
   Useful for enumarating stable releases. Easier to write and read but limited to 256 versions only.

.. envvar:: ENABLE_OTA_ADVANCED

   Default: 0 (disabled)

   If set to 1 the library will work with OtaUpgradeStream which supports signature and encryption of the firmware data itself.
   See :component:`OtaUpgrade` for details. In the application the AdvancedPayloadParser can be used to do the MQTT message handling.

