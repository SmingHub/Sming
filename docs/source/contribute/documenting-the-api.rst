*******************
Documenting the API
*******************

This guide describes how to document the Sming API. It is aimed at
developers of the framework, not users of the framework.

Sming API documentation is created using javadoc style comments within
the source code. Comments are added to header files and doxygen is used
to create HTML documentation. API documentation is organised as a set of
*modules*, each of which represents a logical topic, e.g. Date and Time.

The API documentation covers the core Sming framework and relevant parts
of the ESP SDK (where this is required to use the framework). The API
does not cover contributed libraries which should be documented by the
upstream contributor. Within the source code tree, this includes:

-  Services
-  SmingCore
-  Wiring
-  rboot

The following elements of source code are documented:

-  Global constants
-  Classes
-  Class public members: functions, variables, operators
-  Macros
-  Structures
-  Enumerations

Javadoc comments use the following style:

-  Start with /\*\*
-  End with \*/
-  Each line (between start and end) starts with a space and a \*
-  Doxygen commands are prefixed with @
-  Constants documentation use the command ///<

Each API documentation comment should be part of a module. To define a
module, use the @defgroup command. This should be performed only once
within the Sming framework source code. The format is:

``@defgroup groupname Brief description of the group``

To configure a group to span code, use the @{ and @} commands. When
appended to a comment block, @{ will include all subsequent javadoc
comments until the next @} command. For example:

::

   /** @defgroup DateTime Date and time functions
    * @{
    */
   ...
   /** @brief  Date and time class
   ...
    * @}
    */

To add some documentation comment to a group, use the @addtogroup
command. This may be performed several times within the Sming framework
source code. The format is:

``@addtogroup groupname``

Use the @{ @} commands to span comments. This avoids the need to add
@addtogroup commands to each comment.

To create a hierarchy of groups, use the @ingroup command for child
groups, for example:

::

   /** @defgroup SystemTime System time functions
    *  @ingroup DataTime
    ...

It is common to be able to define a group and span a whole file using
@defgroup and @{ @} commands. This is often possible when all content of
a header file relates to a common topic.

Constant values should be included in the *constants* group, for
example:

::

   /** @brief  ESP GPIO pin configuration
    *  @ingroup constants
    */

To document a function, use the following comment style:

::

   /** @brief  Brief description of function
    *  @param  "Parameter name" Description of parameter (Default: value if defined)
    *  @param  ... rest of parameters
    *  @retval DataType Description of return value
    *  @note   Any extra notes that may assist application developer
    */

For example:

::

   /** @brief  Get human readable date and time
    *  @param  format Select the date format, e.g. dtDateUS for mm.dd.yyyy (Default: dd.mm.yyyy)
    *  @retval String Date and time in format dd.mm.yyyy hh:mm:ss
    *  @note   Date separator may be changed by adding #define DT_DATE_SEPARATOR "/" to source code
    */
       String toFullDateTimeString(dtDateFormat_t format = dtDateFormatEU);

To document a Sming framework constant, use command ///<, for example:

``int8_t DayofWeek; ///< Day of week (0-6 Sunday is day 0)``

To reference another method or function, write its name with ().
Example:

::

   Use `otherMethod()` instead

will match any method in the current class called ``otherMethod``,
regardless of arguments. Note the backticks \` are there just to
highlight the code. If you need to be specific:

::

   Use `otherMethod(const char*, int)` instead

To add a ``See Also`` section, use ``@see``. Example:

::

   @see See `OtherClass()` for details
