Tips and Tricks
===============

.. highlight:: c++

Reading VCC on ESP8266
----------------------
If you are running on a battery operated device then function `system_get_vdd33()` from
the official ESP8266 NONOS SDK can help you read the power voltage.
For the latter to work properly you should make a small change in your application `component.mk`
file and add `vdd` to the `HWCONFIG_OPTS` configuration variable.

If you cannot see such a variable in your `component.mk` file then append the following line to it::

   HWCONFIG := vdd

You can have multiple options selected. They should be separated by comma.
For example the command below will add 4MB flash, spiffs and vdd support::

   HWCONFIG := 4m,spiffs,vdd

You can check the final hardware configuration using the command below::

   make hwconfig

If a custom hardware configuration is needed then read :component:`Storage`.

Minimising Memory Usage
-----------------------

Literals use up memory, so its a good idea to move them to flash.
See :doc:`/framework/core/pgmspace` and :component:`FlashString`.

Webpages and Spiffs
-------------------

:component:`FlashString <FlashString>` turns out to be very useful for sending web pages,
javascript, CSS and so on. Many examples for the ESP8266 exist where a
Spiffs file system is used for this purpose, but in fact Spiffs is not
ideal. If you want to release a new version of your software, and your
web pages are in spiffs, you now have two things to release, so there is
double the chance of something going wrong. Plus you have the challenge
of preserving any user files while refreshing just a few.

One solution is to use a FlashString hooked up to a *FlashMemoryStream*
instead. In the example below, the CSS file is sent compressed to save
time and space. The browser asks for core.js and gets a compressed
version::

   IMPORT_FSTR(flash_corejsgz, PROJECT_DIR "/web/build/core.js.gz")

   void onSendCore_js(HttpRequest &request, HttpResponse &response)
   {
       response.headers[HTTP_HEADER_CONTENT_ENCODING] = _F("gzip");
       auto stream = new FlashMemoryStream(flash_corejsgz);
       response.sendDataStream(stream, MimeType::MIME_JS);
   }

See :component:`FlashString` for further details.

Webpages Performance
--------------------

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
