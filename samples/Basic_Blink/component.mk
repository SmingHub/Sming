## Application Component configuration
## Parameters configured here will override default and ENV values
## Uncomment and change examples:

## Add your source directories here separated by space
# COMPONENT_SRCDIRS := app
# COMPONENT_SRCFILES :=
# COMPONENT_INCDIRS := include

## If you require any Arduino Libraries list them here
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

## Update any additional compiler flags
# CFLAGS +=
# CXXFLAGS +=

## Configure flash parameters (for ESP12-E and other new boards):
# SPI_MODE := dio

## SPIFFS options
DISABLE_SPIFFS := 1
# SPIFF_FILES = files

## Refer to Basic_rBoot sample for options relating to rBoot
