MacOS Host Networking
=====================

Create /dev/tapX devices::

    sudo kextload /Library/Extensions/tap.kext

You will need to confirm the security check then reboot.

Devices are not auto-created, so after rebooting run the above command again.


.. note::

    See https://github.com/ntop/n2n/issues/773.
