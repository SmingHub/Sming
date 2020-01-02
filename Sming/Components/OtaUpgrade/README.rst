Secure Over-the-Air Firmware Upgrade
====================================

This component provides a stream class to read and apply over-the-Air (OTA) firmware upgrades, 
and a utility to generate corresponding upgrade files. 
The upgrade file generation is fully integrated into the Sming build process, such that once you add the dependency on the OtaUpgrade component, Sming will  automatically generate a 'firmware.ota' file as part of the default build target.

The upgrade process is secured by a digital signature, such that only images files with a valid signature are accepted.
