# Sming makefile build system

## Introduction

This guide is provided to assist with understanding, developing and modifying the build system.

A Sming project is built from a set of static libraries (object archives). Typically the application code is one library, built from the user's source code, whilst the other libraries are common to all projects and stored in a separate, shared location.

Until recently Sming itself has always been built as one large library, but this is now broken into a number of discrete Component libraries. The concept is borrowed from Espressif's ESP-IDF build system and whilst it there are similarities the two systems are completely independent.

## Building applications

### Setup

These are the main variables you need to be aware of:

`SMING_HOME` must be set to the full path of the `Sming` directory.

`SMING_ARCH` Defines the target architecture:

* **Esp8266** The default if not specified. `ESP_HOME` must also be provided to locate SDK & tools.

* **Esp32** {todo}

* **Host** builds a version of the library for native host debugging on Linux or Windows

These variables are available for application use:

`PROJECT_DIR` Path to the project's root source directory, without trailing path separator. This variable is available within makefiles, but is also provided as a #defined C string to allow references to source files within application code, such as with the `IMPORT_FSTR` macro.


### Converting existing projects

Instead of `Makefile-user.mk` a project should provide a `component.mk`. To convert to the new style:

1. Copy `Makefile` and `component.mk` from the `Basic_Blink` sample project
2. Copy any customisations from `Makefile-user.mk` into `component.mk` file. (Or, rename `Makefile-user.mk` to `component.mk` then edit it.)
3. Delete `Makefile-user.mk`
4. If the project uses any Arduino libraries, set the `ARDUINO_LIBRARIES` variable

**Targets** You can add your own targets to component.mk as usual. It's a good idea to add a comment for the target, like this:

```
##@Building

.PHONY: mytarget
mytarget: ##This is my target
```

When you type `make help` it will appear in the list.

If you need a target to be added as a dependency to the main application build, add it to `CUSTOM_TARGETS` - the `Basic_Serial` sample contains a simple example of this.

**Arduino Libraries** If your project uses any Arduino libraries, you must set `ARDUINO_LIBRARIES` appropriately.

**Source files** Use `COMPONENT_SRCDIRS` instead of `MODULES`. Use `COMPONENT_SRCFILES` to add individual files.

**Include paths** Use `COMPONENT_INCDIRS` instead of `EXTRA_INCDIR`.

See [component.mk](#component-configuration) for a full list of variables.


### Building

You should normally work from the project directory. Examples:

* Type `make` to build the project and any required Components. To speed things up, use parallel building, e.g. `make -j5` will build using a maximum of 5 concurrent jobs. The optimum value for this is usually (CPU CORES + 1). Using `make -j` will use unlimited jobs, but can cause problems in virtual environments.

* Type `make help` from the project directory to get a list of available build targets.

To switch to a different build architecture, for example:

* Type `make SMING_ARCH=Host` to build the project for the host emulator
* Type `make flash` to copy any SPIFFS image (if enabled) to the virtual flash, and run the application. (Note that you don't need to set SMING_ARCH again, the value is cached.)

To inspect the current build configuration, type `make list-config`.


### Configuration variables

Configuration variables should be set in the project's component.mk file. If appropriate, they can also be set as environment variables.

During development, the easiest way to change variables is on the `make` command line. These are cached so persist between make sessions, and will override any values set in your project's `component.mk` file. For example:

* Type `make SPIFF_BIN=test-rom` to build the project and (if enabled) create a SPIFFS image file called `test-rom.bin`
* Type `make flash COM_PORT=COM4 SPI_MODE=dio SPI_SIZE=4M` to flash the project and `test-rom` SPIFFS image using the provided flash memory settings
* Next time you type `make flash`, the same settings will be used, no need to type them again

A separate cache is maintained for each build type (arch + release/debug). For example:

* Type `make SMING_RELASE=1 list-config` to switch to release build and display the active configuration

Type `make config-clean` to clear all caches and revert to defaults.

For reference, a copy of all build variables are stored in a file with each firmware image created in the 'firmware' directory.


### Component repositories

Placing Components in a common location allows them to be used by multiple projects. To set up your own Component repository, create a directory in a suitable location which will contain your Components and set `COMPONENT_SEARCH_DIRS` to the full path of that directory. For example:

```
	export COMPONENT_SEARCH_DIRS=/opt/shared/Components

	|_ opt
		|_ shared
			|_ Components				# The repository
				|_ MyComponent
				|_ AnotherComponent
				|_ spiffs					# Will be used instead of Sming version
```

User repositories are searched first, which allows replacement of any Component for a project. In this example, our `spiffs` component will be selected instead of the one provided with Sming.  


## Directory layout

The main Sming repo. is laid out like this:

```
Directory/File				Contents
---------	-----				--------

|_ sming
	.appveyor.yml				CI testing (Windows)
	.travis.yml					CI testing (Linux)
	|_ .appveyor					CI scripts (Windows)
	|_ .travis					CI scripts (Linux)
	|_ docs						Sming documentation
	|_ samples					Samples to demonstrate specific Sming features or libraries
	|_ Sming
		Makefile					Builds documentation, performs global actions on the framework
		project.mk				Main makefile to build a project
		build.mk					Defines the build environment
		component.mk				Sming Component definition file
		component-wrapper.mk	Used to build each Component using a separate make instance
		|_ Arch					Architecture-specific makefiles and code
			|_ Esp8266
				sming.mk			Defines architecture-specific Components and libraries
				app.mk				Links the project, create output binaries and perform architecture-specific actions
				build.mk			Architecture-specific build definitions, such as compiler paths
				|_ Compiler
				|_ Components
				|_ Core
				|_ Platform
				|_ System
				|_ Tools			Pre-compiled or scripted tools
			|_ Esp32
				...
			|_ Host
				...
		|_ Components			Framework support code, not to be used directly by applications
		|_ Core					Main framework core
		|_ Libraries				Arduino Libraries
			...
		|_ out						All generated shared files are written here
			|_ Esp8266			The Arch
				|_ debug			The build type
					|_ build		Intermediate object files
					|_ Lib			Generated libraries
					|_ tools		Generated tools
				|_ release
					...
			|_ Host
				...
		|_ Platform				System-level classes
			...
		|_ Services				Modules not considered as part of Core
			...
		|_ System					Common framework low-level system code
			|_ include
		|_ Wiring
			...
	|_ tests						Integration test applications
		...

```

A typical Project looks like this:

```
|_ Basic_Blink
	Makefile					Just calls Makefile-app.mk
	component.mk				Project-specific definitions
	|_ app						Default application source directory
	|_ include				Default application include directory
	|_ out						All generated shared files are written here
		|_ Esp8266			The Arch
			|_ debug			The build type
				|_ build		Intermediate object files
				|_ firmware	Target output files
				|_ Lib			Generated libraries
				|_ tools		Generated tools
			|_ release
				...
		|_ Host
			...
```


## Components

The purpose of a Component is to encapsulate related elements for selective inclusion in a project, for easy sharing and re-use:

* **Shared Library** with associated header files
* **App Code** Source files to be compiled directly into the user's project
* **Header files** without any associated source or library
* **Build targets** to perform specific actions, such as flashing binary data to hardware

By default, a Component is built into a shared library using any source files found in the base or `src` directories. All Arduino Libraries are built as Components. Note that the application is also built as a Component library, but the source directory defaults to `app` instead of `src`.

Components are referred to simply by name, defined by the directory in which it is stored. The Component itself is located by looking in all the directories listed by `COMPONENT_SEARCH_DIRS`, which contains a list of repositories. (Every sub-directory of a repository is considered to be a Component.) If there are Components with the same name in different search directories, the first one found will be used.

Components are customised by providing an optional `component.mk` file.

You can see details of all Components used in a project using `make list-components`. Add `V=1` to get more details.

Note that the application itself is also built as a Component, and may be configured in a similar way to any other Component.

### Library variants

Libraries can often be built using different option settings, so a mechanism is required to ensure that libraries (including the application) are rebuilt if those settings change. This is handled using _variants_, which modifies the library name using a hash of the settings values. Each variant gets its own build sub-directory so incremental building works as usual.

There are several types of config variable:

Variable type | Cached? | Rebuild Component? | Rebuild application? | Relink application
--- | --- | --- | --- | ---
COMPONENT | Y | Y | Y | Y
CONFIG | Y | N | Y | Y
RELINK | Y | N | N | Y
CACHE | Y | N | N | N
DEBUG | N | N | N | N

Variables are usually defined in the context of a Component, in the component.mk file. All Components see the full configuration during building, not just their own variables.

The type of a configuration variable is defined by adding its _name_ to one of the following lists:

`CONFIG_VARS` The Application library derives its variant from these variables. Use this type if the Component doesn't require a rebuild, but the application does.

`COMPONENT_VARS` A Component library derives its variant from these variables. Any variable which requires a rebuild of the Component library itself must be listed. For example, the `esp-open-lwip` Component defines this as `ENABLE_LWIPDEBUG ENABLE_ESPCONN`. The default values for these produces `ENABLE_LWIPDEBUG=0 ENABLE_ESPCONN=0`, which is hashed (using MD5) to produce `a46d8c208ee44b1ee06f8e69cfa06773`, which is appended to the library name.

`RELINK_VARS` Code isn't re-compiled, but libraries are re-linked and firmware images re-generated if any of these variables are changed. For example, `make RBOOT_ROM_0=new-rom-file` rewrites the firmware image using the given filename. (Also, as the value is cached, if you then do `make flashapp` that same iamge gets flashed.)

`CACHE_VARS` These variables have no effect on building, but are cached. Variables such as `COM_SPEED_ESPTOOL` fall into this category.

`DEBUG_VARS` are generally for information only, and are not cached (except for `SMING_ARCH` and `SMING_RELEASE`).


### Dependencies

`COMPONENT_DEPENDS` identifies a list of Components upon which this one depends. These are established as pre-requisites so will trigger a rebuild. In addition, all dependent `COMPONENT_VARS` are (recursively) used in creation of the library hash.

For example, the `axtls-8266` Component declares `SSL_DEBUG` as a `COMPONENT_VAR`. Because `Sming` depends on `sming-arch`, which in turn depends on `axtls-8266`, all of these Components get rebuilt as different variants when `SSL_DEBUG` changes values. The project code (`App` Component) also gets rebuilt as it implicitly depends on `Sming`.


### Component configuration

The `component.mk` is parsed twice, first from the top-level makefile and the second time from the sub-make which does the actual building. A number of variables are used to define behaviour.

These values are for reference only and should not be modified.

`COMPONENT_NAME` Name of the Component
`COMPONENT_PATH` Base directory path for Component, no trailing path separator
`COMPONENT_BUILD_DIR` The current directory. This should be used if the Component provides any application code or targets to ensure it is built in the correct directory (but not by this makefile).
`COMPONENT_LIBDIR` Location to store created Component (shared) libraries
`COMPONENT_VARIANT` Name of the library to build
`COMPONENT_LIBPATH` Full path to the library to be built

These values may be used to customise Component behaviour and may be changed as required.

`COMPONENT_LIBNAME` By default, the library has the same name as the Component but can be changed if required. Note that this will be used as the stem for any variants.
Set `COMPONENT_LIBNAME :=` if the Component doesn't create a library. If you don't do this, a default library will be built but will be empty if no source files are found.

`COMPONENT_TARGETS` Set this to any additional targets to be built as part of the Component, prefixed with `$(COMPONENT_RULE)`. If targets should be built for each application, use `CUSTOM_TARGETS` instead. See `spiffs` for an example.

`COMPONENT_RULE` This is a special value used to prefix any custom targets which are to be built as part of the Component. The target must be prefixed by `$(COMPONENT_RULE)` without any space between it and the target. This ensures the rule only gets invoked during a component build, and is ignored by the top-level make.

`COMPONENT_SUBMODULES` Relative paths to dependent submodule directories for this Component. These will be fetched/patched automatically before building. The patch file should be located in the submodule's parent directory (usually the Component directory itself). If the Component is itself a submodule, then patch files must be placed in `../.patches`. A `.submodule` file is created once the submodule has been fetched and successfully patched.

`COMPONENT_SRCDIRS` Locations for source code relative to COMPONENT_PATH (defaults to ". src")

`COMPONENT_INCDIRS` Include directories available when building ALL Components (not just this one). Paths may be relative or absolute.

`INCDIR` The resultant set of include directories used to build this Component. Will contain include directories specified by all other Components in the build. May be overridden if required.

`COMPONENT_APPCODE` List of directories containing source code to be compiled directly with the application. (Ignore in the project.mk file - use `COMPONENT_SRCDIRS` instead).

`CUSTOM_BUILD` Set to 1 if providing an alternative build method. See [Custom building](#custom-building) section.

`EXTRA_OBJ` Absolute paths to any additional binary object files to be added to the Component archive library.

`COMPONENT_DEPENDS` Set to the name(s) of any dependent Components.

`EXTRA_LIBS` Set to names of any additional libraries to be linked.

`EXTRA_LDFLAGS` Set to any additional flags to be used when linking.

These values are global so must only be appended to (with `+=`) , never overwritten.

`CUSTOM_TARGETS` Identifies targets to be built along with the application. These will be invoked directly by the top-level make.

`GLOBAL_CFLAGS` Use only if you need to provide additional compiler flags to be included when building all Components (including Application) and custom targets.

`APP_CFLAGS` Used when building application and custom targets.


**IMPORTANT NOTE**

During initial parsing, many of these variables (specifically, the `COMPONENT_xxx` ones) _do not_ keep their values. For this reason it is usually best to use simple variable assignment using `:=`.

For example, in `Esp8266/Components/gdbstub` we define `GDB_CMDLINE`. It may be tempting to do this:

```
GDB_CMDLINE = trap '' INT; $(GDB) -x $(COMPONENT_PATH)/gdbcmds -b $(COM_SPEED_GDB) -ex "target remote $(COM_PORT_GDB)"
```

That won't work! By the time `GDB_CMDLINE` gets expanded, `COMPONENT_PATH` could contain anything. We need `GDB_CMDLINE` to be expanded only when used, so the solution is to take a simple copy of `COMPONENT_PATH` and use it instead, like this:

```
GDBSTUB_DIR := $(COMPONENT_PATH)
GDB_CMDLINE = trap '' INT; $(GDB) -x $(GDBSTUB_DIR)/gdbcmds -b $(COM_SPEED_GDB) -ex "target remote $(COM_PORT_GDB)"
```


### Building

For faster builds use make with the `-j` (jobs) feature of make. It is usually necessary to specify a limit for the number of jobs, especially on virtual machines. There is usually no point in using a figure greater than (CPU cores + 1). The CI builds use `-j3`.

Note that `Makefile-app.mk` enforces sequential building to ensure submodules are fetched and patched correctly. This also ensures that only one Component is built at a time which keeps the build logs quite clean and easy to follow.

Components can be rebuilt and cleaned individually. For example:

* `make spiffs-build` runs the Component 'make' for spiffs, which contains the spiffs library and spiffy tool.
* `make spiffs-clean` removes all intermediate build files for the Component
* `make spiffs-rebuild` cleans and then re-builds the Component

By default, a regular `make` performs an incremental build on the application, which invokes a separate (recursive) make for the `App` Component. All other Components only get built if any of their targets don't exist (e.g. variant library not yet built). This makes application building faster and less 'busy', which is generally preferable for regular application development. For Component development this behaviour can be changed using the `FULL_COMPONENT_BUILD` variable (which is cached). Examples:

* `make FULL_COMPONENT_BUILD=lwip` will perform an incremental build on the `lwip` Component
* `make FULL_COMPONENT_BUILD=1` will incrementally build all Components


### Custom Building

To use an external makefile or other build system (such as CMake) to create the Component library, or to add additional shared libraries or other targets, customise the `component.mk` file as follows:

1. Set `CUSTOM_BUILD=1`
2. Define the custom rule, prefixed with `$(COMPONENT_RULE)`. Note that Components are built using a separate make instance with the current directory set to the build output directory, not the source directory.

It is important that the rule uses the provided values for `COMPONENT_LIBNAME`, `COMPONENT_LIBPATH` and `COMPONENT_LIBDIR` so that variant building, cleaning, etc. work correctly. See below under 'Building', and the Host `lwip` Component for an example.

Components are built using a make instance with the current directory set to the build output directory, not the source directory. If any custom building is done then these variables must be obeyed to ensure variants, etc. work as expected:

`COMPONENT_LIBNAME` as provided by component.mk, defaults to component name, e.g. `Sming`
`COMPONENT_LIBHASH` hash of the component variables used to create unique library names, e.g. `13cd2ddef79fda79dae1644a33bf48bb`
`COMPONENT_VARIANT` name of the library to be built, including hash. e.g. `Sming-13cd2ddef79fda79dae1644a33bf48bb`
`COMPONENT_LIBDIR` directory where any generated libraries must be output, e.g. `/home/user/sming/Sming/out/Esp8266/debug/lib/`
`COMPONENT_LIBPATH` full path to the library to be created, e.g. `/home/user/sming/Sming/out/Esp8266/debug/lib/clib-Sming-13cd2ddef79fda79dae1644a33bf48bb.a`
`COMPONENT_BUILDDIR` where to write intermediate object files, e.g. `/home/user/sming/Sming/out/Esp8266/debug/build/Sming/Sming-13cd2ddef79fda79dae1644a33bf48bb`

### Porting existing libraries

to be completed


## Known Issues

**Cleaning** Components are not cleaned unless defined. e.g. `make axtls-8266-clean` will fail unless you also specify `ENABLE_SSL=1`.

**Empty libraries** Components without any source code produce an empty library. This is because, for simplicity, we don't want to add a component.mk to every Arduino library.

**Empty Component directories** Every sub-directory in the `COMPONENT_SEARCH_DIRS` is interpreted as a Component. For example, `spiffs` was moved out of Arch/Esp8266/Components but if an empty directory called 'spiffs' still remains then it will be picked up instead of the main one. These sorts of issues can be checked using `make list-components` to ensure the correct Component path has been selected.

**Components as submodules** All component.mk files must be available for parsing. If they are contained in a GIT submodule then that must be fetched first. Some settings may be specified in a components.mk file instead. 

