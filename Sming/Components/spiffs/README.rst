SPIFFS IFS Library
==================

This Component provides SPIFFS filesystem support for all architectures.

A single SPIFFS partition is defined using :envvar:`HWCONFIG` ``=spiffs``, which supports these build variables:

   .. envvar:: DISABLE_SPIFFS

      [deprecated and removed]

      This value is no longer supported. Please remove it from your project's component.mk file.


   .. envvar:: SPIFF_SIZE

      [deprecated and removed]

      Size (in bytes) of the SPIFFS area in Flash memory. To change this, edit the :ref:`hardware_config`.

   .. envvar:: SPIFF_FILES

   .. envvar:: SPIFF_FILES

      default: ``files``

      The SPIFFS image is built using files from this directory, which must exist or the build will fail.

      If you set this to an empty value, then an empty filesystem will be created.


   .. envvar:: SPIFF_BIN

      Filename to use for the generated SPIFFS filesystem image. The default is ``spiff_rom``.


   .. envvar:: SPIFF_BIN_OUT

      [read-only] Shows the full path to the generated image file.


For more control over the SPIFFS partition you can create your own partition definition in a
custom :ref:`hardware_config`.



.. envvar:: SPIFF_FILEDESC_COUNT

   Default: 7

   Number of file descriptors allocated. This sets the maximum number of files which may be opened at once. 


.. envvar:: SPIFFS_OBJ_META_LEN

   Default: 16

   Maximum size of metadata which SPIFFS stores in each file index header (after the filename).
   If this value is changed, existing SPIFFS images will not be readable.

   The default value given here is provided to support :component:`IFS` extended file attribute information.

   The first 16 bytes are used for system attributes (e.g. modified time), so setting this to, say, 64
   leaves 48 bytes for user metadata. Each attribute has a 2-byte header (tag + size) so a single user
   attribute can be stored of up to 46 bytes, or multiple tags up to this limit.

   Note: :library:`LittleFS` provides better support for user metadata.

