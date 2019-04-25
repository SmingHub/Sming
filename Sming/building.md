# Sming makefiles

## Introduction

This guide is provided to assist with developing and modifying the build system.

There are two main makefiles:

> **Makefile** to build the Sming framework libraries

> **Makefile-app.mk** to build an application

There are also specific makefiles for each supported host environment:

> **Makefile-bsd.mk**

> **Makefile-linux.mk**

> **Makefile-macos.mk**

> **Makefile-windows.mk**

The following are internal makefiles containing shared script for both framework and application:

> **build.mk** Defines the build environment

> **modules.mk** Parses defined MODULES, etc. and creates target rules

In addition, `Sming/Platform/$(PLATFORM)` must contain:

> **sming.mk** to define platform-specific modules and libraries

> **app.mk** to build an application

`PLATFORM` may be defined as

> **Esp8266**

> **Esp32** {todo}

> **Host** {todo} builds a version of the library for native host debugging

If setting this as an environment variable you should use `SMING_PLATFORM` to avoid potential conflicts.

## Directories

```
Directory				Contents
---------				--------
Libraries\			Arduino Libraries
Platform/
	Common/			Headers and code common to all platforms
	Esp8266/			Existing Sming platform code
		Compiler/
		Core/
		out/
		Platform/
		System/
		Tools/
	Esp32/
		...
	Host/
		...

SmingCore/			Main framework core (TODO: rename to `Core`)

System/				Common framework system code
	esp8266/
	include/
	
third-party/			Shared GIT submodules

Wiring/

```


## Environment Setup

You must provide `SMING_HOME` and `ESP_HOME`. All others are optional.

# Building the framework

## Makefile

Targets include:

> **all** (default) Builds the Sming library, user libraries, arduino libraries and tools

> **libsming** Build the Sming framework and user libraries

> **mqttc** Build mqttc-codec user library

> **docs** Build the documentation

> **third-party** Fetch third-party submodules, but do  not build

> **libraries** Fetch Arduino libraries from repos, but do not build

> **samples** Build all the sample applications

> **test** Build a few basic test applications

> **tools** Build all required internal tools

Esp8266 targets:

> **axtls** Build axtls-8266 library - `ENABLE_SSL=1` required

> **pwm_open** Build the PWM replacement library

> **pwm-clean**

Cleaning:

> **dist-clean** Clean everything

> **clean** Remove intermediate files

> **user-lib-clean** Clear generated user libraries

> **third-party-clean** Reset state of third-party repos

> **libraries-clean** Reset state of Arduino libraries

> **samples-clean** Invoke **clean** on all samples

> **tools-clean**

> **cs** Apply coding style to core files

Flags:

> ** ARDUINO_LIBRARIES** Specify which Arduino Libraries are to be built. Default: unspecified, builds them all. Example: `make ARDUINO_LIBRARIES="ArduinoJson SI7021 BMP180"`
This can be set in the platform `sming.mk` file.

## GIT Submodules

These are used in various places throughout the framework. A single `%/.submodule` rule takes care of updating and optionally patching the code.
A patch file with the same name as the submodule must be present in `..` or `../.patches`.
After updating and patching, an `.submodule` file is created to confirm this has been done.

# Building applications

## Makefile-app.mk

This does not contain any targets, but sets up the common build environment, configures Sming variables then invokes the platform-specific makefile in `Platform$/$(PLATFORM)/app.mk`

## Application Code (APPCODE)

Some code is recompiled for every application (e.g. `gdbstub`, `rboot`).
Code must be placed in a separate sub-directory of the corresponding module or component, then added to the `APPCODE` variable within the makefiles.
This mechanism is appropriate for files with multiple configurable options and avoids the need to rebuild the framework.

# Known Issues

## Parallel build

i.e. Using `make -j` doesn't work for rules which pull in submodules from GIT, so this must be done as a first step:

`make third-party libraries`

then you can

`make -j`

to compile the framework or application. Ensure that any options (`ENABLE_SSL`, `ARDUINO_LIBRARIES`) are set identically for both.


## Cleaning

Optional libraries are not cleaned unless defined. e.g. `make axtls-clean` will fail unless you also specify `ENABLE_SSL=1`.


## Custom heap

Changing the heap allocator requires a rebuild of the framework.
TODO: Implement as separate user library.

