Coding Style Rules
==================

.. highlight:: c++

The benefits of coding standards are readability, maintainability and
compatibility. Any member of the development team in Sming should be
able to read the code of another developer. The developer who maintains
a piece of code tomorrow may not be the coder who programmed it today.

Therefore we enforce coding standards as described in this guide.
The coding style rules are mandatory for most of the framework,
and Pull Request that does not adhere to the coding style rules will not
be merged until those rules are applied.

The rules are optional for libraries, but recommended.

Tools will help you adhere to these coding standards without the need to know them by heart.
See :doc:`clang-tools` for further details.

Please also bookmark the `C++ Core Guidelines <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines>`__.
This is an invaluable reference for writing good code and making the best use of this powerful language.



Indentation
-----------

We use tabs for indentation. Configure your editor to display a tab as
long as 4 spaces. For reference, the corresponding settings in clang-format are::

   TabWidth:        4
   UseTab:          Always
   IndentWidth:     4



Naming
------

Classes, Structures, type aliases

   Must be nouns in UpperCamelCase, with the first letter of  every word capitalised.
   Use whole words — avoid acronyms and abbreviations (unless the abbreviation is much more widely
   used than the long form, such as URL or HTML).
   
   Examples::

      class HttpClient {}

      class HttpClientConnection {}

      using LargeValue = uint32_t;

      struct MyStruct {
         ...
      };

      enum MyEnum {
         a, ///< Comment if required
         b,
         c,
      };

   .. note::
   
      The trailing , on the final item in an enumeration declaration will ensure that clang-format
      places each item on a separate line. This makes for easier reading and the addition of line
      comments if appropriate.



Methods

   Must be either verbs in lowerCamelCase, or a multi-word name that begins with a verb in lowercase;
   that is, with the first letter lowercase and the first letters of subsequent words in uppercase.
   
   Examples::

      bind();

      getStatus();


Variables

   Local variables, instance variables, and class variables must also be written in lowerCamelCase.
   Variable names must not start with, end with or contain underscore (\_) or dollar sign ($) characters.
   This is in contrast to some coding conventions which prefix all instance variables with underscore,
   however this is reserved by the C++ standard and can create problems.

   Variable names should be short yet meaningful. The choice of a variable name should be mnemonic — that is,
   designed to indicate to the casual observer the intent of its use. One-character variable names should be avoided except for
   temporary “throwaway” variables. Common names for temporary variables are i, j, k, m, and n for integers; c, d, and e for characters.
   
   Examples::

      int i;

      char c;                  

      WebsocketClient* client;


Pre-processor definitions

   #defined macros must be written in uppercase characters separated by underscores.
   Names may contain digits if appropriate, but not as the first character. For example::

      #define MAX_PARTICIPANTS 10


Constants

   Typically declared using ``const`` or ``constexpr`` and, like variables, should be lower-camelcase.
   Names **MUST NOT** be all-uppercase as these may be confused with #defined values.

   See `C++ Core Guidelines <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#enum5-dont-use-all_caps-for-enumerators>`__.


Use of ``typedef`` in C++
-------------------------

Use of ``typedef`` in C++ code is not recommended.

The `using` keyword has been available since C++11 and offers a more natural way to express type definitions.
It is also necessary in certain situations such as templating.

For example::

   using ValueType = uint32_t;

is more readable than::

   typedef uint32_t ValueType;

Especially in multiple type declarations the subject is always immediately after the ``using`` keyword
and makes reading much easier.

https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#t43-prefer-using-over-typedef-for-defining-aliases

https://www.nextptr.com/tutorial/ta1193988140/how-cplusplus-using-or-aliasdeclaration-is-better-than-typedef

enum/struct declarations
------------------------

This::

   typedef struct _MyStructTag {
     ...
   } MyStruct;
   
   typedef enum _MyEnumTag {
     ...
   } MyEnum;
   
is overly verbose and un-necessary. It's a hangover from 'C' code and should generally be avoided for readability and consistency.
This is the preferred definition::

   struct MyStruct {
     ...
   };
   enum MyEnum {
   .............
   };

It's also un-necessary to qualify usage with `enum`. i.e. `MyEnum e;` is sufficient, don't need `enum MyEnum e;`.


.. highlight:: text

C++ Standard
------------

For the moment we recommend the use of C++11. The corresponding settings in clang-format are::

   Standard:        Cpp11
   Cpp11BracedListStyle: true

Starting and ending spaces
--------------------------

We don’t recommend the use of a starting or ending space in angles,
container literals, c-style cast parentheses, parentheses and square
brackets. Our settings are::

   SpaceAfterCStyleCast: false
   SpaceBeforeParens: Never
   SpaceInEmptyParentheses: false

   SpacesInAngles:  false
   SpacesInContainerLiterals: false
   SpacesInCStyleCastParentheses: false
   SpacesInParentheses: false
   SpacesInSquareBrackets: false

See the meaning of those keys and their selected values in the
`ClangFormatStyleOptions document <http://releases.llvm.org/5.0.0/tools/clang/docs/ClangFormatStyleOptions.html>`__.

Line length
-----------

We are living in the 21st century so most of the monitors should be
capable of displaying 120 characters per line. If a line is longer than
those characters it will be split whenever possible::

   ColumnLimit:     120

Empty Lines
-----------

Two or more empty lines will be compacted to one. Also we delete empty
lines at the start of a block::

   KeepEmptyLinesAtTheStartOfBlocks: false
   MaxEmptyLinesToKeep: 1

Braces
------

See the meaning of these keys and their selected values in the
`ClangFormatStyleOptions document <http://releases.llvm.org/5.0.0/tools/clang/docs/ClangFormatStyleOptions.html>`__::

   BraceWrapping:
       AfterClass:      false
       AfterControlStatement: false
       AfterEnum:       true
       AfterFunction:   true
       AfterObjCDeclaration: false
       AfterStruct:     false
       BeforeElse:      true
       IndentBraces:    false
   BreakBeforeBraces: Linux


Pointer Alignment
-----------------

Always on the left::

   PointerAlignment: Left

Includes
--------

We don’t re-sort includes although it is highly recommended to order the
headers alphabetically whenever possible::

   SortIncludes:    false

Comments
--------

We try not to split comment lines into smaller ones and also we add one
space between code and trailing comment::

   ReflowComments: false
   SpacesBeforeTrailingComments: 1

Spaces
------

For readability put always spaces before assignment operators::

   SpaceBeforeAssignmentOperators: true

Standard file headers
---------------------

Please use the standard Sming header with copyright notice::

   /****
    * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
    * Created 2015 by Skurydin Alexey
    * http://github.com/anakod/Sming
    * All files of the Sming Core are provided under the LGPL v3 license.
    *
    * [Insert filename here] - [optional brief description of file]
    *
    * @author [date] [name] [email]
    *
    * [comments]
    *
    ****/

Do not include details of minor changes to the file as this is handled
by GIT. It may be appropriate to add notes to identify major changes or
contributions. These should be marked with a new @author tag.

Deprecating code
----------------

Where a change in the Sming API may break existing users’ code, then the
existing type/method/function/variable must be maintained for a time to allow
time for migration to the new technique. Such changes should only be
made if there is a good reason, for example improved reliability,
performance, ease of use.

Deprecation requires two steps:

Step 1: Add a ``@deprecated`` tag to the method header comment so the change
is flagged in the auto-generated API documentation. Include a brief
explanation of the new method or technique to be adopted. See also
`Documenting the API <https://github.com/SmingHub/Sming/wiki/Documenting-the-API>`__.

Example::

   /** @deprecated Use `anotherMethod()` instead */

Step 2: Append ``SMING_DEPRECATED`` to the method declaration so the
compiler will flag a warning if that method is used during compilation.

The framework and samples must build without referencing any deprecated
methods, functions or variables.

Virtual Classes
---------------

Sming makes extensive use of virtual classes. If you are modifying or
adding virtual methods then please follow these guidelines:

**Rule**: The base class must have a virtual destructor, even if it
doesn’t do anything. Example::

   virtual ~Stream() {}


**Rule**: Inherited classes must not prepend ``virtual`` or append
``override`` to any destructor. Example::

   ~IDataSourceStream();

Rationale: virtual destructors do not behave like regular virtual
methods - they are ‘chained’ rather than overridden - therefore
``override`` is not appropriate and ``virtual`` is both un-necessary
and unhelpful


**Rule**: Use the ``override`` directive on inherited virtual methods::

   int read() override;

Rationale: The compiler will ensure there is actually a base method to
inherit from and generate a warning if one is not found, or if
parameters do not correspond.


**Rule**: Don’t use empty destructors in inherited virtual classes

Rationale: They’re not necessary



Common issues
-------------

Some notes on commonly occurring issues::


   /**
     * @brief Basic example class
     */
   class VirtualBuffer
   {
   public:
       virtual ~VirtualBase
       {
       }

       virtual unsigned getLength() const = 0;
   };

   /**
     * @brief Descendant example class
     */
   class MemoryBuffer : public VirtualBuffer
   {
   public:
       /*
           Note: Omit destructor if not required in descendant
       */
       ~VirtualDescendant()
       {
           /*
               Note: delete includes null pointer check so you don't have to
           */
           delete buffer;
       }

       /*
           Use `const` qualifier for methods which don't modify object
        */
       const char* getBuffer() const
       {
           return pos;
       }
       
       /*
           Trivial code should go into the class header file where possible.
           Rationale: Compiler is better able to optimise code. Easier to read.

           Use `override` on virtual methods
       */
       unsigned getLength() const override
       {
           return length;
       }

       /*
           Use methods to access member variables rather than making them public
           Rationale: Protects data, helps when tracking down bugs
       */  
       void setBuffer(char* newBuffer, unsigned newLength)
       {
           delete buffer;
           buffer = newBuffer;
           length = newLength;
       }

   private:
       /*
           Each class should operate on a small, well-defined item of data.
       */


       /*
           Class variables should be defined with initialisers, rather than using code in the constructor.
           Rationale: Reduces/eliminates risk of un-initialised data causing unpredictable behaviour.
       */
       char* buffer = nullptr;

       /*
           Remember `int` can be unsigned! If a value doesn't need to be signed, don't make it so.
           Rationale: unsigned values are simpler to check, less likely to introduce bugs, compiler can better optimise computations
       */
       unsigned length = 0;
   };


