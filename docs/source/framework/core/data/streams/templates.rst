Template Streams
================

.. highlight:: c++

Sming provides several classes to assist with serving dynamic content:

Basic Templating
----------------

The :cpp:class:`TemplateStream` class is a stream which performs variable-value substitution using
``{varname}`` style markers, which are replaced as the stream is read.

You can find a simple demonstration of how this class is used in the
:sample:`HttpServer_Bootstrap` sample application.

.. note::

    There must be no whitespace after the opening brace.
    For example, ``{ varname }`` will be emitted as-is without modification.

    This allows inclusion of CSS fragments such as ``td { padding: 0 10px; }`` in HTML
    without resorting to double-braces.

    If necessary, use double-braces ``{{varname}}`` in your template and
    call :cpp:func:`TemplateStream::setDoubleBraces` ``(true)``.

    Invalid tags, such as ``{"abc"}`` will be ignored, so JSON templates do not require
    special treatment.


Variable values can be set using :cpp:func:`TemplateStream::setVar` or :cpp:func:`TemplateStream::setVars`.
These are stored in a :cpp:class:`HashMap` which can be accessing directly via :cpp:func:`TemplateStream::variables`.

To support calculated values and external lookups, an optional callback may
be provided via :cpp:func:`TemplateStream::onGetValue`.
This is invoked only if a variable is not found in the map.

Another option is to use TemplateStream as a base class and override the :cpp:func:`TemplateStream::getValue` method.

.. important::

    If required, text must be escaped appropriately for the output format.
    For example, encoding reserved HTML characters can be handled using :cpp:func:`Format::Html::escape`.


Advanced Templating
-------------------

Introduction
~~~~~~~~~~~~

The :cpp:class:`SectionTemplate` class extends :cpp:class:`TemplateStream` to provide more advanced dataset processing capabilities.
It is intended to be used as the base class for a data provider.

One such implementation is the :cpp:class:`IFS::DirectoryTemplate` class.
The :sample:`Basic_IFS` sample demonstrates how it can be used to provide a formatted directory
listing in multiple formats, using a different template for each format.

The :sample:`Basic_Templates` sample illustrates a similar appraoch using data from CSV data files.

If the output format requires escaping, create an instance of the appropriate :cpp:class:`Format::Formatter`
and call :cpp:func:`SectionTemplate::setFormatter`.
If providing custom values via callback, obtain the current formatter via :cpp:func:`SectionTemplate::formatter`
class and call the ``escape`` method.
Note that for performance reasons this is not done automatically as often variable values
do not require escaping. User-provided values or filenames must always be properly escaped.


Sections
~~~~~~~~

Templates typically contain multiple sections.
The :cpp:class:`IFS::DirectoryTemplate`, for example, uses 3 sections for header, content and footer.
The header and footer are emitted exactly once, but the content section is repeated for each available data record.

The :cpp:class:`SectionStream` class is used internally so that all sections can be provided within a single file.

Sections are (by default) marked ``{SECTION}`` ... ``{/SECTION}``.
Everything outside of these markers is ignored, so can contain comments.


Using SectionTemplate
~~~~~~~~~~~~~~~~~~~~~

Implementations should provide the following methods:

nextRecord
    This method is called before a new content record is about to be output.
    Here's the annotated :cpp:class:`IFS::DirectoryTemplate` implementation::

        // Return true if we have a new valid record, false if not
        bool nextRecord() override
        {
            // Content section we fetch the next directory record, if there is one
            if(sectionIndex() == 1) {
                return directory->next();
            }

            // This code emits the header and footer sections exactly once
            // Returning false suppresses their output completely
            return recordIndex() < 0;
        }

    This sets up the 'current' directory information record.


getValue
    Lookup values for a given field::

        String getValue(const char* name) override
        {
            // return ...
        }

    .. important::

       If required, text must be escaped appropriately for the output format.
       Use :cpp:func:`SectionTemplate::formatter` to obtain the current
       For example, encoding reserved HTML characters can be handled using :cpp:func:`Format::Html::escape`.


Control language
~~~~~~~~~~~~~~~~

A basic control language is implemented using ! escaped tags.
Commands may have zero or more arguments, separated by ``:``.

- Numbers must be decimal and start with a digit, e.g. ``11`` or ``5.6``
- Strings must be quoted "..."
- Sub-expressions must be contained in braces {...}

Anything else is treated as a variable name.
Variable names beginning with $ are reserved for internal use.
The following values are currently defined:

``$section`` The current section index
``$record`` The current record index

Conditional if/else/endif statements may be nested.

This is the current command list:

- ``{!int:A}`` Output A as integer
- ``{!float:A}`` Output A as float
- ``{!string:A}`` Output A as quoted string
- ``{!mime_type:A}`` Get MIME type string for a filename
- ``{!replace:A:B:C}`` Copy of A with all occurrences of B replaced with C
- ``{!length:A}`` Number of characters in A
- ``{!pad:A:B:C}`` Copy of A padded to at least B characters with C (default is space). Use -ve B to left-pad. C
- ``{!repeat:A:B}`` Repeat A, number of iterations is B
- ``{!kb:A}`` Convert A to KB
- ``{!ifdef:A}`` emit block if A is not zero-length
- ``{!ifdef:A}`` emit block if A is zero-length
- ``{!ifeq:A:B}`` emit block if A == B
- ``{!ifneq:A:B}`` emit block if A != B
- ``{!ifgt:A:B}`` emit block if A > B
- ``{!iflt:A:B}`` emit block if A < B
- ``{!ifge:A:B}`` emit block if A >= B
- ``{!ifle:A:B}`` emit block if A <= B
- ``{!ifbtw:A:B:C}`` emit block if B <= A <= C
- ``{!ifin:A:B}`` emit block if A contains B
- ``{!ifin:A:B}`` emit block if A does not contain B
- ``{!else}``
- ``{!endif}``
- ``{!add:A:B}`` A + B
- ``{!sub:A:B}`` A - B
- ``{!goto:A}`` move to section A
- ``{!count:A}`` emit number of records in section A
- ``{!index:A}`` emit current record index for section A

.. note::

   See :source:`Sming/Core/Data/Streams/SectionTemplate.h` for an up-to-date list of commands and internal variables.

Here's an excerpt from the Basic_IFS sample, displaying information for a single file:

.. code-block:: html

   {!iflt:$record:100} <!-- If $record < 100 -->
       <tr>
           <td>{$record}</td>
           <td>{file_id}</td>
           <td><a href="{path}{name}"><span style='font-size:20px'>{icon}</span> {name}</a></td>
           <td>{!mime_type:name}</td>
           <td>{modified}</td>
           {!ifin:attr:"D"} <!-- Value of 'attr' variable contains "D" ->
               <td></td><td></td>
           {!else}
               <td>{size}<br>{!kb:size}&nbsp;KB</td>
               <td>{original_size}<br>{!kb:original_size}&nbsp;KB</td>
           {!endif}
           <td>{!replace:attr_long:", ":"<br>"}</td>
           <td>{compression}</td>
           <td>{access_long}</td>
       </tr>
   {!else} <!-- $record >= 100 -->
       Too many records {$record}
   {!endif}



API Reference
-------------

.. doxygenclass:: TemplateStream
   :members:

.. doxygenclass:: SectionTemplate
   :members:

.. doxygenclass:: SectionStream
   :members:

