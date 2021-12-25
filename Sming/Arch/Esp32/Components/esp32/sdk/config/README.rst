config
======

Default SDK project settings are contained in several files,
which are concatenated into the project's ``sdkconfig.default`` file.

The following files are assembled, in order:

    common
    {BUILD_TYPE}
    {SMING_SOC}.common
    {SMING_SOC}.{BUILD_TYPE}
    {PROJECT_DIR}/{SDK_CUSTOM_CONFIG}

Where:

-  ``{BUILD_TYPE}`` is ``debug`` or ``release``.
-  ``SDK_CUSTOM_CONFIG`` is optional and is always relative to the project root directory.

To provide additional settings for a specific variant use the variant prefix such
as ``esp32.debug``, ``esp32c3.release``, ``esp32s2.common``, etc.

etc.
