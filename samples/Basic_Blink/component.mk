## Application Component configuration
## Parameters configured here will override default and ENV values
## Uncomment and change examples:

## If appropriate, declare which SOCs your project supports
# COMPONENT_SOC :=

## If project doesn't require networking, saves RAM and build time
DISABLE_NETWORK := 1

## Add your source directories here separated by space
# COMPONENT_SRCDIRS := app
# COMPONENT_SRCFILES :=
# COMPONENT_INCDIRS := include

## If you require any Libraries list them here
# ARDUINO_LIBRARIES :=

## List the names of any additional Components required for this project
# COMPONENT_DEPENDS :=

## Set paths for any GIT submodules your application uses
# COMPONENT_SUBMODULES :=

## Append any targets to be built as dependencies of the project, such as generation of additional binary files
# CUSTOM_TARGETS += 

## Additional object files to be included with the application library
# EXTRA_OBJ :=

## Additional libraries to be linked into the project
# EXTRA_LIBS :=

## Provide any additional compiler flags
# COMPONENT_CFLAGS :=
# COMPONENT_CXXFLAGS :=

## Configure hardware
# Default is 'standard' (no spiffs), can also provide your own
#HWCONFIG := spiffs

## Select source of content for default `spiffs` partition when built
# SPIFF_FILES = files

## Refer to Basic_rBoot sample for options relating to rBoot
