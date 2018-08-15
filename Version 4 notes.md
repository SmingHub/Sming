# Sming Framework Version 4.0 draft

Having found a framework which does many things the 'right way', with an excellent design philisophy, there are some global issues which I feel require attention.

## Background

My requirement was for a processor to base a series of mains-powered controllers on,
including a simpler push-button user interface with LCD or LED display. I chose the
ESP-12F because of:

	* Cost - major advantage, very hard to ignore this even with ESP32
	* Integrated WiFi
	* Flash memory size
	* Power consumption: for mains power consumption is very low
	* Adequate RAM
  	
Balanced against these 'shortcomings':
  
	* Limited RAM (vs. ESP32)
	* Low pin count (vs. ESP32 or Silicon Labs offerings)
	* No way to protect source code; with an open-source approach this isn't really a consideration
	* No way to write-protect critical flash areas 
  
The low pin count simply means most peripherals need to be SPI/I2C/RS232 connections, which turned out to be acceptable.

Choosing the devlopment platform was not so straightforward. We have:
  
	* Non-OS SDK - the basis for Arduino and Sming, C-based
	* Espressif OpenRTOS SDK - C-based kernel with wide support, standard architecture
	* Arduino - C++ framework devised as a learning platform for AVR microcontrollers. This has evolved to support more advanced hardware than the initial AVR devices but is still bound to its core polling architecture and code is often littered with 'delays' which just eat CPU cycles.
	* Sming - sets out from the start to provide asynchronous operation whilst maintaining backward-compatibility with Arduino libraries and applications.

	Whilst going down the OpenRTS route seemed like the sensible appropach, it is still just a framework built on top of the non-OS SDK. And it's C-based. I could therefore see the merit in starting with something fresh, embracing object-oriented programming and the idiosyncracies of the ESP8266. It was also a definite plus not having the complication of pseudo-threading.
	An event-based system seemed more appropriate given the hardware constraints.

 
## V4 motivation  

There are a number of 'features' I've developed which I'd like to to be considered for integration into the core Sming framework. However, they do require some fairly significant improvements to the framework, hence the 'V4 proposal' tag.

	### Installable File System (new feature)

		Removes dependency on SPIFFS at the application level so any supported file system can be used. Also adds things like file timestamps, access control, file attributes.
		Includes a very lightweight read-only 'Firmware' file system, with a simple Python build script. I use this in a 'hybrid' filesystem which layers SPIFFS over the fixed files.
		The existing fileXXX functions remain the core API for this.

  	### System date/time management (update)

		Converting between UTC and local time needs improvements to support timezones accounting for daylight savings. This is available in Arduino libraries, as is sunrise/sunset calcuation (based on geographic co-ordinates), but both of these need to be integrated into the framework.
		
	### RF Switch (rewrite)
	
		This arduino library is a good example of what we need to avoid in Sming as it uses delays to perform timing. I've rewritten it using the Hardware timer and callbacks.

  	### IO Control subsystem (new feature)
  	
  	A mechanism for queueing requests asynchronously to hardware devices. This currently implements modbus and RF Switch devices. Three object classes are used:

  		* Controller: the hardware which talks over RS485, to an RF transmitter, etc. There is one controller instance for each physical device available. The controller serialises requests to the hardware and invokes a callback at the start of execution and on completion.
  		* Device: an instance of a modbus slave or RF switch transmitter. These sit on top 
  		of a controller to define characteristics or protocols specific to the device or slave.
  		* Request: Encapsulates a command request for a specific device class.

This is an IO _Control_ system because it was not written for bulk data transfer, however it could easily support this. A potential application is for providing an asynchronous file system API to access bus-based devices, such as hardware SPI (where transfers are performed via interrupts in the background) or even network storage. The benefit would be much simpler applications as ultimately everything could be handled using streams.

This is all nothing new of course: FreeRTOS, for example, uses a structure of device drivers to abstract the hardware from higher implementation layers.


## Summary of impact

Changes are widespread so I've had to do this as a major revision. Clearly there is the risk of introducing bugs, but with cleaner code these should be easier to track down.

### Network responsiveness

In general I've found the network more responsive and the annoying 'hangs' seem to be a thing of the past. This is quite subjective, of course; it would be interesting to see how others fare.

### RAM

There's more RAM available. For example, MeteoControl builds like this on the standard framework (3.6.1.2) with SMING_RELEASE=1:

   Section|                   Description| Start (hex)|   End (hex)|Used space
      data|        Initialized Data (RAM)|    3FFE8000|    3FFE83CD|     973
    rodata|           ReadOnly Data (RAM)|    3FFE83D0|    3FFEABEC|   10268
       bss|      Uninitialized Data (RAM)|    3FFEABF0|    3FFF1A00|   28176
      text|            Cached Code (IRAM)|    40100000|    40106B02|   27394
irom0_text|           Uncached Code (SPI)|    4020A000|    4025143E|  291902
Total Used RAM : 39417
Free RAM : 42503
Free IRam : 5392

On V4 we get this:

   Section|                   Description| Start (hex)|   End (hex)|Used space
      data|        Initialized Data (RAM)|    3FFE8000|    3FFE8489|    1161
    rodata|           ReadOnly Data (RAM)|    3FFE8490|    3FFE91D0|    3392
       bss|      Uninitialized Data (RAM)|    3FFE91D0|    3FFEFEF8|   27944
      text|            Cached Code (IRAM)|    40100000|    401067EA|   26602
irom0_text|           Uncached Code (SPI)|    40208000|    4025179A|  300954
Total Used RAM : 32497
Free RAM : 49423
Free IRam : 6184


### Significant API changes

#### IDataSourceStream

Method changes affect all class which inherit
	* virtual StreamType getStreamType() const
	* virtual size_t readMemoryBlock(char* data, size_t bufSize)

#### URL

Path member variable no longer exposed, use path() method
relativePath() method added to remove leading '/' character

#### SystemClass 
 
_onReady_ methods will invoke callback immediately if system is ready. Previously the callback would never have been invoked.

_deferCallback_ method added to make use of global task queue (HardwareSerial class rewritten internally to use this).

#### Debug printing / m_printf

Removed the dependency on Hardware serial so applications may redirect output to other places.


## Build environment

I develop on Windows using Eclipse. I use linux for Raspberry Pi work but as yet
I haven't a dedicated Linux machine but at some point I'll set up a virtual machine to check builds on Linux.

I haven't investigated the following aspects of Sming:
	Chocolatey
	Automated testing

## Checks

Sming libraries build in SSL and non-SSL with no warnings or errors, except in Libraries - some have issues.
All Sample applications build successfully.

## Testing framework

There does not appear to be a standard way of testing the framework on a development system (linux / Windows, for example).
I built a 'SmingW' library which pulls in as much of the framework as possible to build tools and test code using MinGW. It uses alternative headers to deal with low-level definitions and platform-specifics but this is fairly minimal.

@todo Add this library to the repository. Currently builds under eclipse so need to do a makefile for it.

## Code review

### Makefiles

There is much similarity between the makefiles for building the Sming framework and for the RBOOT and non-RBOOT application builds.

@todo Look at using refactoring makefiles to eliminate duplication.

Many compiler warnings have been turned off, some quite important ones. These have been re-enabled.

Symbols no longer stripped from release object files; this does not affect firmware does but does allow the MemAnalyzer to work. It is not clear why symbol stripping is required at all as it can be of some assistance with faultfinding in release builds.

### Module coupling

Modules should only be dependent upon those actually required to do the task at hand.
Unwanted dependencies bloat firmware.

### Include files

A .h file uses the minimum #includes required for the definitions in the header (not the source).
Where type from non-trivial #include is only accessed by reference, consider placing a forward reference instead. e.g. if including a member variable HardwareSerial*, just place class HardwareSerial at the top rather than #include "HardwareSerial.h"

Again, with code modules only the minimum number of #includes to be used.

Avoid too low-level #includes, use <user_config.h> or "WiringFrameWorkIncludes.h" for example.

Whilst just including "SmingCore.h" in everything might be easy, anything in SmingCore or Services should not be doing this. Such instances have been replaced with targeted includes. Same applies to arduino.h, arduinocompat.h, and so on.

@todo Get a definitive list of non-preferred header files.
@todo guidelines for selecting the right header file to use, and the path

### Class headers

Trivial code moved into header files, except where it causes dependency or other compiler issues. This benefits optimisation of code size and speed and also reduces code size and complexity and simplifies maintenance. A disadvantage is that it triggers rebuilds of more modules.

One class, one header (and optional source) file. SmingCore/DataStream/DataSourceStream is a big example of what not to do; it has been split into separate files.

### Visual verification

Code should be as simple as possible so it can be easily understood by visual inspection. It otherwise hides bugs and other problems.

### Class member visibility

Member variables should not be declared as public. Instead, use accessor methods. There was a lot of this in the HttpConnection, HttpServerConnection, SmtpClient, TcpClient, HttpResponse and HttpRequest classes which has been eliminated. Instead, methods have been added to operate on the member data instead so code has been moved around. This has also eliminated some duplication.

### Class variables

These have all been renamed with leading underscore to differentiate from parameters. It is way to easy to confuse these with parameters.

### Data initialisation

Cannot rely on class instances or other data being zero-initialised, so default values must be assigned to all class variables. This is preferable to doing it in the constructor.
Makes constructor code simpler and eliminates risk of un-initialised values.

### Naming conventions

Names are all camel case. Variables start with a lower case, types with an upper case.

### Compiler warnings

Code must build with all compiler warnings enabled, and treated as errors.
Makefile has been commented with further details of this.

### Library or Service?

Need some guidelines as to where modules should be placed. It seems that Libraries are for Arduino code, whereas Services are for customised or custom Sming-specific code that doesn't fit in SmingCore.

Are Services only integrated into Sming? Is there any dependency on external repositories?

### Libraries

Code quality is hugely variable. There should be two library folders, one for high quality code and another for everything which probably needs some attention. Makefiles should be updated so user code isn't broken but if I wish to build Sming with only the good libraries then it makes that much easier.

ArduinoJson, for example, is a high quality module which would be classed as a 'core library'. Attributes for a HQ module would include:
	* Proven track record
	* Written to a good standard (even if it's not the same as the Sming standard)
	* No timer-driven polling; code should execute asynchronously using callbacks to perform specific tasks, except where alternatives are not available. For example, polling for keypresses via SPI or I2C might be necessary if there is no interrupt line available.
	* No delay loops or calls to watchdog timer: both of these violate the core principle of Sming.
	* Ideally, written to be cross-platform so it can be tested under linux/mingw build.

It would be preferable to treat compiler warnings as errors, however many of the libraries would fail to build.

@todo Perhaps 'unapproved' libraries should be built into a separate library, against libsming.


### Module-private functions/data

Where functions and data are meant only for use within a module they must be declared static. This avoids unintended conflicts with other modules.

### Global class instances

These should be at the top of a source file with other variables, not the bottom.
The generally accepted ordering is:

	#includes
	type definitions
	constant data
	variable data
	local functions
	class methods


### Signed/unsigned

Try to avoid expressions mixing signed and unsigned values. Keep these to a minimum - the compiler warns because it's doing a conversion 'under the hood' which is usually not intended. Turning the warning off or casting is not good practice.

Most instances have been changed to use unsigned (or size_t) values. This also simplifies range checks since a < 0 check isn't required.

This is very important. There is an example in m_printf.cpp around line 179:

		size_t len;
		int precision = -1;
		...
		if (len > precision)
	
	Should be:
		if (precision >= 0 && (int)len > precision)

	It could mean:
		if (len > (unsigned)precision)
	or
		if ((int)len > precision)

	The former works, and is the intended action. The latter crashes the CPU because it sets len to -1, or 0xFFFFFFFF.

### Character values

Use of char 0 replaced with '\0' (semantics).

### Strings

Number of instances where String parameters are passed instead of 'const String&'.

Instead of subString(), remove() is preferred to avoid un-necessary heap allocation and copy.
For example, s.remove(0, 1) is equivalent to s = s.subString(1)

There are instances where potentially large data is excessively buffered in order to get it into a String. String::setLength() has been added to size the string buffer and allow data to be written into it directly, for example via fileRead().

The String class has been modified internally to use memmove() instead of strcpy() operations. This allows a String to contain any kind of data, not just nul-terminated strings.

Where "" is returned for a string value, this frequently means 'value not provided' or 'unknown'. In these situations nullptr is more appropriate to indicate 'undefined'. It also allows use of the 'if (str)' expression which will evaluate to false. The default constructor for String has been changed to nullptr from "". (This exposed a missing null _buffer check which has been fixed.)

Two const static data members have been added to String so they may be used where a const reference is required. i.e. const String& return values.

@todo Consider adding support for double-null terminated string lists (sz-arrays). They can be a useful alternative to arrays of string pointers.


### A note on PROGMEM

A feature of the ESP8266 is the need to access flash memory in aligned 32-bit chunks. The AVR processors using non-harvard architecture access code and data on separate busses so require the well-documented PROGMEM feature to store this. The ESP8266 has it's fakepgmspace module to do a similar job but place such definitions into a read-only flash segment to avoid having them copied into RAM at startup. Such strings are emitted into the .rodata segment by the linker.

There is the mforce-l32 GCC compiler mod. to enforce the correct instruction usage, that doesn't seem to be available for the standard xtensa toolkit and requires a rebuild. I also recall this conflicts with other compiler mods.

### String and constant data

All text and constant data should be expliclitly marked with PROGMEM (directly or using helper macros) and accessed using appropriate macros or functions. This is good practice for a resource-limited system since it gives greater control over code generation. It text requires translation into other languages this can be automated more easily if appropriate macros are used. It also allows the framework to be used with AVR processors if required.

Support has been added to debug_progmem.h and the String class to assist with this. The _F() and F() macros (similar to those in Arduino) both define a flash string in-situ and pull it into a buffer. _F() uses a buffer on the stack, whereas F() uses a String object so the data is on the heap. The former is preferable for functions requiring a C-type string. Note that because the size (length) of the string is known at compile time a call to strlen_P() is not required. The string buffer is also word-aligned (rounded up to a multiple of 4 bytes) so the memcpy_P operation is optimised. The new memcpy_aligned function has been added to make this more efficient.

Note that multiple instances of regular constant (.rodata) data can be optimised by the compiler into a single instance. For example, "\r\n" has been left as-is because it's very common.

However, the compiler doesn't appear to do this for these macros because the data is declared static to allow it to be defined within functions. Where strings and other constant data are used multiple times it is good practice to define these at the top of the source file and refer to them, rather than placing them directly into the code. It is preferable to do this for all string data.

The technique I've adopted is to define these strings as function calls. Two macros are used to do this:

	DEFINE_STRING_P(_name, _str)
		Define a function which returns a String object with the string data
	DECLARE_STRING_P(_name)
		Declare a prototype for the function, used in header files

### Lists and enumeration

There  are instances where a C++ wrapper is required around existing data structures. There are at least two instances where a Vector is not the best way to achieve this:
	1. FileSystem.cpp - used to create a directory listing
	2. Station.cpp - used in a callback with an AP list

In (1) using the standard opendir(), readdir(), closedir() is simple enough and far more efficient, plus it provides all the required information. This becomes more apparent with the IFS feature update which provides this as a standard API, including standard file metadata.

In (2) we only need a single object to wrap the structure returned by the system. Methods interpret the data only as needed, with reset() and next() methods. Additional methods can be provided to take static copies of the information, if required, either individually or for the entire data set. There is an opportunity here to introduce a standard 'enumerator' class for these sorts of things.

### Use of auto

This is an awesome C++ feature. When used judiciously it makes code much more readable and reduces the need for debugging variable types. It's usually obvious from context what the variable type is so saves some typing as well.

### Commented-out code

Removed. Or, if appropriate replaced with a conditional debug expression and/or commented to explain why it's there.

### Use of 'friend'

Refactor and remove where appropriate. This violates class encapsulation and makes code less robust. It's really only appropriate for small helper classes defined in the same module; using this across separate modules is a bad idea.

### Missing function prototypes

Espressif SDK has some missing function prototypes which generate warnings. These have been included in an upate to the esp-open-lwip.patch file.

### NULL vs nullptr

Usage in C++ code replaced with nullptr.
Setting pointers to '0' is not acceptable, replaced with nullptr.

### IDataSourceStream / ReadWriteStream

The intent of these two classes is clear: IDataSourceStream is read-only operations. The uses of these in the code was quite convoluted in places. See HttpResponse for an example.

### Error return checking

To check return values of functions/methods returning negative error codes, use 'if (ret >= 0)' to check for success, and 'if (ret < 0)' for failure.
Expressions such as 'if (ret != -1)' do not catch out-of-range values.
It may be prudent to define global functions/macros for this e.g. SUCCESS(_x) and FAILED(_x)

### Magic numbers

Never return 'magic numbers' from functions; these must be defined in a header file with explanation of their purpose.

@todo Ideally instances of functions returning -1 could be replaced with a  global error value.

### unsigned / unsigned int

These are equivalent so 'unsigned' is preferred for brevity. As with 'int' we use these types where the exact size is not important to allow the compiler more freedom.

### Macros vs. inline functions

In C++ static inline functions are preferable to macros. They offer type checking and can be overloaded or templated.

### Use of 'constexpr'

Whilst I've not used this, I did come across it so could be useful to introduce it where appropriate. When used with static inline functions, prompts the compiler to resolve all values at compile time or throw an error.
Helps to eliminate unintentional code bloat and complements 'force inline'.

### Use of 'force inline'

Generally not necessary; compiler optimisation settings should ensure these are inlined correctly.

### HTTP Parser

Should we be using http_should_keep_alive() function instead of checking for "close" in headers?

Apart from on_headers_complete, there does not appear to be any requirement for specific error values returned from callbacks, they just need to be non-zero. The HPE_xxx error code identifies which callback failed, but not the specific code.

### Default method/function parameters

Propagated default values from header into definition as comments removed.
This is not helpful, just clutters the code.

### Timers

OSTimer class added to wrap OS timer functions. Used in preference to the main Timer class where the additional features of the main Timer class are not required. This reduces code complexity and RAM usage.

### Task queue

Global task queue required to simplify and streamline interrupt handlers.
  
### Closures / lambdas

A major benefit of C++. In many instances allows setup and callback code to live together which makes following the execution flow easier and helps with maintainability.
