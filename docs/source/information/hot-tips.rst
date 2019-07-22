*********
Hot Tips!
*********

.. highlight:: c++

Minimising Memory Usage
=======================

Literals use up memory, so its a good idea to move them to flash. Be
aware though that it can be slower! Further reading, see the header
FlashString.h in the sming source code

F() macro
---------

Instructs the compiler to store the literal in flash (of
which there is plenty) rather than RAM (which is limited).

Example:

::

   String system = F("system");

_F() macro
----------

This function uses the stack, and is generally preferable
to the ``F()`` macro, but you need to be mindful of scope. The content is
lost as soon as the containing block goes out of scope. Used as a
function parameter, that means the end of the function call.

Examples:

::

   println(_F("Debug started"));

   commandOutput->print(_F("Welcome to the Tcp Command executor\r\n"));


Bad:

::

   char* s = _F("string")

An assignment such as this will not work because the temporary will be
out of scope after the statement, hence s will point to garbage. In this
instance PSTR_ARRAY(s, “string”) can be used.

PSTR_ARRAY() macro
------------------

This macro creates and loads string into a named stack buffer.
This Ensures loaded string stays in scope, unlike _F() Example:

::

   String testfunc() {
      //char * test = "This is a string"; <<- BAD
      PSTR_ARRAY(test, "This is a string");
      m_printf(test);
      ...
      return test;
   }

JSON keys
---------

Example:

::

   root[F("offset")] = something;

Bad:

::

   root[_F("offset")] = something;

According to the ArduinoJson docs it should take an internal copy of
char* strings, but it doesn’t! Tip - Use the F() macro without leading
underscore instead.

Webpages and Spiffs
===================

FlashString turns out to be very useful for sending web pages,
javascript, CSS and so on. Many examples for the ESP8266 exist where a
Spiffs file system is used for this purpose, but in fact Spiffs is not
ideal. If you want to release a new version of your software, and your
web pages are in spiffs, you now have two things to release, so there is
double the chance of something going wrong. Plus you have the challenge
of preserving any user files while refreshing just a few.

One solution is to use a FlashString hooked up to a FlashMemoryStream
instead. In the example below, the CSS file is sent compressed to save
time and space. The browser asks for core.js and gets a compressed
version.

::

   IMPORT_FSTR(flash_corejsgz, "web/build/core.js.gz")

   void onSendCore_js(HttpRequest &request, HttpResponse &response)
   {
       response.headers[HTTP_HEADER_CONTENT_ENCODING] = _F("gzip");
       auto stream = new FlashMemoryStream(flash_corejsgz);
       response.sendDataStream(stream, MimeType::MIME_JS);
   }

Webpages Performance
====================

HTML markup can get quite large and the bigger the file the slower the
page loads. One way to deal with that is to remove the white space, this
process is called minifying. The downside is that the result is
difficult for a human to read. I recommend against it, at least in the
early stages of your project.

To support the HTML files there are CSS files and JS files, which must
be kept locally on the server if one wants things to work even when the
internet is absent.

I use the bootstrap library and the CSS I write goes into another
special file. The file count is now three, an HTML file and two CSS
files. This is already a lot of files for a microcontroller to deal with
especially if it gets download requests for all three at once. A browser
will start a download request for each file it sees, and for the ESP,
any more than three is a problem, meaning we need to keep this under
control.

One way to deal with that is to combine the CSS files together into one.

Next we have JavaScript files which includes the custom code, the
bootstrap library and the jquery library. Three extra files. Once again
we can deal with these by combining them into one, in which We are back
to having 3, one HTML file one CSS file and one JavaScript file.

But the files are big and this is a problem not just because it is slow.
The watchdog does not like things to take a long time, and you will
almost certainly end up with a timeout.

When a browser asks for a file it doesn’t mind receiving a compressed
version using gzip. (Note that you need to add “Content-Encoding/gzip”
to the header in the response from the server). Using gzip vastly
reduces the sizes of files and it’s well worth doing.

Another size optimisation for CSS files is to remove unused CSS (UNCSS)
- I recommend against this as it was too aggressive at removing stuff I
really needed - YMMV.

I use gulp to automate the extraction and concatenation and compression
of the CSS and JS files, here is the relevant part of my gulpfile.js:

.. code-block:: js

   function htm() {
      return gulp.src(htmConfig.src)
         .pipe(useref())
         .pipe(gzip())       // compresses to a gzip file
         .pipe(size({ showFiles: true }))
         .pipe(gulp.dest('web/build/'))
    }

My webpage looks like this

.. code-block:: html

     <!-- build:css core.css -->
     <link rel="stylesheet" type="text/css" href="bootstrap.css">
     <link rel="stylesheet" type="text/css" href="style.css">
     <!-- endbuild -->

After gulp runs it looks like this

.. code-block:: html

     <link rel="stylesheet" href="core.css">
