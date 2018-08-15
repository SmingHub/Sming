# Version 4 proposed additional changes

Remove items from this list as they are added to the framework.

## Station.h / Station.cpp

Provide alternative method for enumerating BssInfo. It's inefficient and clunky.
The list is a linked list so requires only a single object to wrap the data, with accessor methods to translate as required. Add a next() method to get the next entry, and a reset() method to start again.

Consider whether we need to retain support for the existing mechanism.

NB. There may be other instances where a Vector is just lazy programming. We've already deprecated one in FileSystem.cpp :-)

  
## DateTime / Timezone / SolarCalculator

Add format() method for dates and times.
Provide built-in selection for default formats, e.g.
    AM/PM or 24-hour
    d/m/y or m/d/y
    etc.
Optimise. parseHttpDate, for example.
  
    
## IOControl subsystem

Consider how to integrate this. Would it be better to pass configuration using something other than JSON ? Callback, perhaps ?
  
## EndlessMemoryStream

Establish where this is used and document to tighten up specification. Suspect it's only used for chunked transfers. Could be useful for performing transfers from an IORequest callback.
  
## Authentication

FTPServer has it's own method for login/password storage. There is an implicit need for a general authentication module which deals with all this. It can maintain such information and perform authentication, returning UserRole values, etc. The class will be overridable to perform custom application stuff.
  
## SZ-strings

Add support to String class for double-null-terminated strings. e.g.
	String sz = 
		"value 1\0"
		"value 2\0"
		"value 3\0";
	sz.getText(0);	// Returns "value 1"
	sz.getValue("value 3");	// Returns 2

## Code/string mapping

To obtain a mapping between enumerated values and strings is a little involved, think about how to provide a more generic way to do this.


## Internationaliation support

Provide a standard mechanism for implementing strings which may require language alternatives.
This could be as simple as defining macros for such strings, e.g. _T() instead of _F() perhaps. These would be emitted to a separate section.
This problem has doubtless been addressed by FreeRTOS or other systems so shouldn't have to re-invent the wheel here.


## File system management tool

Write a tool based on IFS to manipulate file system images from any of the supported file systems. For example, FWFS, SPIFFS and FAT.

Initially a command-line tool would be appropriate. We could use this to remove the SPIFFS dependency from the build, instead using a separate makefile for filesystems integration.

A graphical front end would be nice. Perhaps a C++ library back-end with a python GUI ?

