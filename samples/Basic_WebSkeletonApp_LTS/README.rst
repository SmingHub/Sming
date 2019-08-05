Basic Web Skeleton (LTS)
========================

Introduction
------------

This is a copy of the *Basic_WebSkeletonApp* from the Sming LTS branch, modified to build for Sming 4.0.
Sming uses ArduinoJson version 6 by default, so this sample demonstrates how to continue using version 5.

This is how it was done:

1. When including SmingCore.h, just use `#include <SmingCore.h>`.
2. Remove the file `include/user_config.h`. Sming provides this by default, but if you have made changes
   for your project then remove everything except your changes, but be sure to add `#include_next <user_config.h>` at the top.
3. Remove any `#include <user_config.h>` statements from your files. This file is automatically included by Sming.
4. Rename `Makefile-user.mk` file to `component.mk`.
5. Replace the file `Makefile` with the one from the `Basic_Blink` sample project. If you've ignored the instructions
   and modified the file (!) then you'll need to move those changes into your new `component.mk` file instead.
6. Sming uses `#pragma once` style for header guards, so consider updating your own files if you're not already doing this.
7. We use the `OneWire` Arduino Library, so add `ARDUINO_LIBRARIES := OneWire` to our component.mk file.
   Sming now only builds libraries which are specifically requested for a project.
8. We're using ArduinoJson, version 6. Sming supports version 5 or 6, but you must specify which one:

   * Add `ARDUINO_LIBRARIES=ArduinoJson6` to the project's component.mk file.
   * Add `#include <JsonObjectStream.h>`. If you're not using the stream class, add `#include <ArduinoJson6.h>` to code.
9. Update callback function parameter lists for ``STADisconnect`` and ``STAGotIP``.
   See :source:`Sming/Platform/StationClass.h`.

That's it.
