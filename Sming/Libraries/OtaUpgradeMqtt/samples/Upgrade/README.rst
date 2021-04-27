OTA over MQTT
=============

.. highlight:: bash

Introduction
------------

This example demonstrates how you can create an application that updates its firmware Over The Air (OTA) using the MQTT protocol.
This application uses :library:`OtaUpgradeMqtt` and follows the recommended versioning principles.

Based on :envvar:`ENABLE_OTA_ADVANCED` the firmware data can be either without any encoding or be signed and encrypted.

Tools
-----
There are two tools that facilitate the packiging and deployment of a new firmware.

For more information read ``Firmware packaging`` in the documentation of the :library:`OtaUpgradeMqtt` component.

Security
--------
Depending on :envvar:`ENABLE_SSL` a standard SSL/TLS can be enabled. This way

1. The communication between the application and the server will be encrypted using standard SSL.

2. To prove that the server is the correct one: The MQTT clients should pin the public key fingerprint on the server.
   OR have a list of public key fingerprints that are allowed.

3. Depending on :envvar:`ENABLE_CLIENT_CERTIFICATE` the application can send a client certificate that is signed by the server.

Configuration
-------------

.. envvar:: APP_ID

   Default: "test"

   This variable contains the unique application name.

.. envvar:: APP_VERSION

   Default: not set

   Contains the application major and minor versions separated by comma. Example "4.2".
   If not set will use the current major and minor version from Sming.

.. envvar:: APP_VERSION_PATCH

   Default: not set

   Contains the application patch version as integer. For stable versions you can use 0 until 255.
   For unstable versions the current timestamp can be used as a patch version.

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
   See :library:`OtaUpgrade` for details.

.. envvar:: ENABLE_SSL

   Default: unset (disable)

   If set to 1 (highly recommended), OTA upgrade files will be trasnferred securely over TLS/SSL.

.. envvar:: ENABLE_CLIENT_CERTIFICATE

   Default: 0 (disabled)

   Used in combination with ``ENABLE_SSL``. Set to 1 if the remote server requires the application to authenticate via client certficate.

.. envvar:: MQTT_URL

   Default: depends on  ``ENABLE_SSL`` and ``ENABLE_CLIENT_CERTIFICATE`` values

   Url containing the location of the firmware update MQTT server.
