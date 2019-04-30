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

In addition, `Sming/Arch/$(ARCH)` must contain:

> **sming.mk** to define architecture-specific modules and libraries

> **app.mk** to build an application

`SMING_ARCH` may be defined as

> **Esp8266**

> **Esp32** {todo}

> **Host** {todo} builds a version of the library for native host debugging (e.g. Linux/Windows)


## Directories

```
Directory				Contents
---------				--------

Arch/					Architecture-specific makefiles and code
	Esp8266/
		Compiler/
		Components/
		Core/
		out/
		Platform/
		System/
		Tools/
	Esp32/
		...
	Host/
		...

Components/			Framework support code, not to be used directly by applications

Core/					Main framework core

Libraries/			Arduino Libraries

Platform/				System-level classes

Services/				Modules not considered as part of Core

System/				Common framework low-level system code
	esp8266/
	include/
	
Wiring/

```


## Environment Setup

You must provide `SMING_HOME` and any architecture-specific ones. For the Esp8266 you must define `ESP_HOME` for SDK & tools.

# Building the framework

## Makefile

Flags:

> ** ARDUINO_LIBRARIES** Specify which Arduino Libraries are to be built. Default: unspecified, builds them all. Example: `make ARDUINO_LIBRARIES="ArduinoJson SI7021 BMP180"`
This can be set in the platform `sming.mk` file.

## GIT Submodules

These are used in various places throughout the framework. A single `%/.submodule` rule takes care of updating and optionally patching the code.
A patch file with the same name as the submodule must be present in `..` or `../.patches`.
After updating and patching, an `.submodule` file is created to confirm this has been done.

# Building applications

## Makefile-app.mk

This does not contain any targets, but sets up the common build environment, configures Sming variables then invokes the platform-specific makefile in `Platform$/$(PLATFORM)/app.mk`.

## Application Code (APPCODE)

Some code is recompiled for every application (e.g. `gdbstub`, `rboot`).
Code must be placed in a separate sub-directory of the corresponding module or component, then added to the `APPCODE` variable within the makefiles.
This mechanism is appropriate for files with multiple configurable options and avoids the need to rebuild the framework.

# Known Issues

## Parallel build

i.e. Using `make -j` doesn't work for rules which pull in submodules from GIT, so this must be done as a first step:

`make submodules`

then you can

`make -j`

to compile the framework or application. Ensure that any options (`ENABLE_SSL`, `ARDUINO_LIBRARIES`) are set identically for both.


## Cleaning

Optional libraries are not cleaned unless defined. e.g. `make axtls-clean` will fail unless you also specify `ENABLE_SSL=1`.


## Custom heap

Changing the heap allocator requires a rebuild of the framework.
TODO: Implement as separate user library.

