*****************************
Over-the-Air Firmware Upgrade
*****************************

This component offers a writable stream that decodes and applies [Over-the-Air] firmware upgrade files, 
as well as a small python utility to generate those upgrade files as part of Sming's build process.
It may be combined with any transport mechanism that is compatible with Sming's stream classes.
Check out the :sample:`HttpServer_FirmwareUpload` example, which demonstrates how the integrated HTTP server can be 
used to provide a web form for uploading new firmware images from the browser.

Prerequisites
=============

Every in-system firmware upgrade mechanism for ESP-based devices requires partitioning the 
flash into two slots: One slot holds the currently running firmware, while the other slot receives the upgrade.
As a consequence only half of the available flash memory can be used for the firmware. (Actually a bit less because
a few sectors are reserved for the bootloader and various parameter blobs.)

In most cases, it is sufficient to set :envvar:`RBOOT_ROM1_ADDR` to the offset address of the second slot.
See the :component:`rboot` documentation for further options and considerations.
If your partitioning choice results in two ROM images being created, they are transparently combined such that there
is always a single OTA upgrade file. During the upgrade, the OTA code will automatically select the right image and 
ignore the one for the other slot.

.. attention::
   Make sure that the ROM slots do not overlap with each other or with areas of the flash allocated
   to other purposes (file system, RF calibration parameters, etc.). Sming will *not* detect 
   a misconfigured flash layout.

Security features leverage :library:`libsodium`, which is automatically pulled in as a dependency when signing and/or
encryption is enabled. You also have to **install libsodium bindings for python** on your development computer, either 
using ``python -m pip install PyNaCl`` (recommended for Windows users) or, if your are on Linux, preferably via your 
distribution's package manager (search for a package named 'python-nacl').

Usage
=====

The ``OtaUpgradeStream`` class
------------------------------

The component provides a single class :cpp:class:`OtaUpgradeStream`, which derives from :cpp:class:`ReadWriteStream`, 
but, despite its base class, is only a writable stream.

At construction time, the address and size of the slot to receive the new firmware is automatically determined from the 
rBoot configuration. No further setup required. Just feed the OTA upgrade file into the 
``OtaUpgradeStream::write`` method in arbitrarily sized chunks. The flash memory is updated on the fly as data arrives
and upon successful validation, the updated slot is activated in the rRoot configuration.

Once the file is complete, call ``OtaUpgradeStream::hasError`` to check for any errors that might have occurred during 
the upgrade process. The actual error, if any, is stored in the public member ``OtaUpgradeStream::errorCode`` and can be 
converted to an error message using ``OtaUpgradeStream::errorToString``.
In addition, you may also examine the return value of the ``OtaUpgradeStream::write`` method, which will be equal to the
given chunk size, unless there is an error with the file or the upgrade process.

Building
--------

The component is fully integrated into the Sming build process. Just run 

::

   make
   
and find the OTA upgrade file in ``out/<arch>/<config>/firmware/firmware.ota``. 
If security features are enabled but no secret key file does exist yet, a new one is generated during the first build. 
You may change it later by modifying :envvar:`OTA_KEY` or using the :ref:`ota-rollover-process`.

Now install the OTA-enabled firmware once via USB/Serial cable and you are all set to do future upgrades wirelessly over
your chosen communication channel.

A convenience target 

::

   make ota-upload OTA_UPGRADE_URL=http://<your-ip>/upgrade

is provided for the not too uncommon use case of uploading the OTA file as a HTTP/POST request (but obviously is of no 
value for other transport mechanisms). The URL is cached and can be omitted from subsequent invocations.


Configuration and Security features
===================================

.. envvar:: OTA_ENABLE_SIGNING

   If set to 1 (the default and highly recommended), OTA upgrade files are protected against unauthorized modification 
   by a digital signature. This is implemented using libsodium's ``crypto_verify_...`` API, which encapsulates a public 
   key algorithm: A secret (or 'private') signing key never leaves the development computer, while a non-secret 
   ('public') verification key is embedded into the firmware. Public key algorithms cannot be broken even if an attacker 
   gains physical access to one of your devices and extracts the verification key from flash memory, because only 
   someone in possession of the secret signing key (see :envvar:`OTA_KEY`) is able to create upgrade files with a valid 
   signature.

   .. note::
   
      You may disable signing in order to save some program memory if your communication channel already establishes a 
      comparable level of trust, e.g. TLS with a pinned certificate.

.. envvar:: OTA_ENABLE_ENCRYPTION
   
   Set to 1 to enable encryption of the upgrade file using libsodium's ``crypto_secretstream_...`` API, in order to 
   protect confidential data embedded in your firmware (WiFi credentials, server certificates, etc.).
   
   It is generally unnecessary to sign encrypted upgrade files, as encryption is also authenticating, i.e. only someone 
   in possession of the secret encryption key can generate upgrade files that decrypt successfully.
   There is, however, one catch: Unlike signing, encryption *can* be broken if an attacker is able to extract the
   decryption key (which is identical to the encryption key) from flash memory, in which case all current and future
   file encrypted with the same key are compromised. Moreover, the attacker will be able to generate new valid upgrade 
   files modified to his or her agenda. Hence, **you should only every rely on encryption if it is impossible for an 
   attacker to gain physical access to your device(s)**. But otherwise, you shouldn't have stored confidential data on 
   such device(s) in the first place!
   Conversely, you should *not* encrypt upgrade files that do not contain confidential data, to avoid the risk of 
   accidentally exposing a key you might want to reuse later. For this reason, encryption is disabled by default.
   
   Note: To mitigate a catastrophic security breach when the encryption key is revealed involuntarily, encryption and 
   signing can be enabled at the same time. This way, an attacker (who probably has access to your WiFi by now) will at
   least be unable to take over more devices wirelessly. But keep in mind: it is still not a good idea to store 
   confidential data on an unsecured device!

   Note also that the described weakness is not a property of the selected encryption algorithm, but a rather general 
   one. It can only be overcome by encrypting the communication channel instead of the upgrade file, e.g. with TLS, 
   which uses a key exchange protocol to negotiate a temporary encryption key that is never written to flash memory. But 
   even then, it is still unwise to embed confidential data into the firmware of a device that is physically accessible
   to an attacker - now you have been warned!

.. envvar:: OTA_KEY

   Path to the secret encryption/signing key. The default is ``ota.key`` in the root directory of your project. If the 
   key file does not exist, it will be generated during the first build. It can also be (re-)generated manually using the
   following command (usually as part of a :ref:`ota-rollover-process`):
   
   ::

      make ota-genkey

   The key file must be kept secret for obvious reasons. In particular, set up your .gitignore (or equivalent VCS 
   mechanism) carefully to avoid accidentally pushing the key file to a public repository.
   
   By pointing ``OTA_KEY`` to a shared location, the same key file can be used for multiple projects, even if their 
   security settings differ, since the key file format is independent of the security settings. (In fact, it is just a 
   string of random numbers, from which the actual algorithm keys are derived.)

.. envvar:: OTA_ENABLE_DOWNGRADE

   By default, ``OtaUpgradeStrem`` refuses to downgrade to an older firmware version, in order to prevent an attacker 
   from restoring already patched security vulnerabilities. This is implemented by comparing timestamps embedded in the
   firmware and the upgrade file. To disable downgrade protection, set OTA_ENABLE_DOWNGRADE to 1.
   
   Downgrade protection must be combined with encryption or signing to be effective. A warning is issued by the build 
   system otherwise.
   

.. envvar:: OTA_UPLOAD_URL

   URL used by the ``make ota-upload`` command.

.. envvar:: OTA_UPLOAD_NAME

   Field name for the upgrade file in the HTTP/POST request issued by ``make ota-upload``, corresponding to the ``name``
   attribute of the HTML input element:

   .. code-block:: html

      <input type="file" name="firmware" />

   The default is "firmware".

.. _ota-rollover-process:

Key/Settings rollover process
=============================

There might be occasions where you want to change the encryption/signing key and or other OTA security settings (e.g. 
switch from signing to encryption or vice versa). While you could always install the new settings via USB/serial cable,
you can also follow the steps below to achieve the same goal wirelessly:

#. Before modifying any security-related settings, start the rollover process by issuing
   ::

      make ota-rollover

#. Now modify security settings as desired, e.g. generate a new key using ``make ota-genkey``.

#. Run ``make`` to build a rollover upgrade file. The firmware image(s) contained in this file use the new security 
   settings, while the upgrade file itself is created with the old settings (saved by the command in step 1) and thus is
   still compatible with the firmware currently running on your device(s).

#. Upgrade wirelessly using the rollover file created in step 3. The new security settings are now installed.

#. Finalize the rollover process using the command
   ::

      make ota-rollover-done

   This will delete temporary files created by step 1.


.. _ota-file-format:

OTA upgrade file format
=======================

Basic file format
-----------------

The following layout is used for unencrypted upgrade files, as well as for the data inside the encrypted container 
(see next paragraph). All fields are stored in little-endian byte order.

+--------------------+-------------------------------------------------------------------------------+
| Field size (bytes) | Field description                                                             |
+====================+===============================================================================+
| 4                  | | Magic number for file format identification:                                |
|                    | | ``0xf01af02a`` for signed images                                            |
|                    | | ``0xf01af020`` for images without signature                                 |
+--------------------+-------------------------------------------------------------------------------+
| 8                  | OTA upgrade file timestamp in milliseconds since 1900/01/01                   |
|                    | (used for downgrade protection)                                               |
+--------------------+-------------------------------------------------------------------------------+
| 1                  | Number of ROM images (1 or 2)                                                 |
+--------------------+-------------------------------------------------------------------------------+
| 3                  | reserved, always zero                                                         |
+--------------------+-------------------------------------------------------------------------------+
| variable           | ROM images, see below                                                         |
+--------------------+-------------------------------------------------------------------------------+
| | 64 (signed)      | | With signature: Digital signature over the whole file up to this point.     |
| | 16 (otherwise)   | | Otherwise: MD5 HASH over the whole file up to this point. This is           |
|                    |   not a security measure but merely protects the integrity of the file. MD5   |
|                    |   was selected, because it already available in the ESP8266's on-chip ROM.    |
+--------------------+-------------------------------------------------------------------------------+

Each ROM image has the following format:

+--------------------+-------------------------------------------------------------------------------+
| Field size (bytes) | Field description                                                             |
+====================+===============================================================================+
| 4                  | Start address in flash memory (i.e. :envvar:`RBOOT_ROM0_ADDR` for first ROM)  |
+--------------------+-------------------------------------------------------------------------------+
| 4                  | Size of ROM in bytes                                                          |
+--------------------+-------------------------------------------------------------------------------+
| variable (see      | ROM image content                                                             |
| previous field)    |                                                                               |
+--------------------+-------------------------------------------------------------------------------+

More content may be added in a future version (e.g. SPIFFS images, bootloader image, RF calibration data blob). 
The reserved bytes in the file header are intended to announce such additional content.

Encryption Container format
---------------------------

Encrypted files are stored in chunks suitable for consumption by libsodium's ``crypto_secretstream_...`` API.

The first chunk is always 24 bytes and is fed into ``crypto_secretstream_pull_init`` to initialize the decryption
algorithm.

Subsequent chunks are composed of:

 * A 2 byte header indicating the length of the chunk minus 1.
   The default chunk size used by otatool.py is 2 kB.
 * The data of the chunk, which is fed into ``crypto_secretstream_pull``.

For further information on the data stored in the header and the chunks, refer to libsodium documentation and/or source 
code.
