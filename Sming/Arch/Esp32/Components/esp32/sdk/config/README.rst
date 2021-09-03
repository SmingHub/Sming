config
======

Default SDK project settings are contained in several files,
which are concatenated into the project's ``sdkconfig.default`` file.

The following files are assembled, in order:

    common
    {BUILD_TYPE}
    {ESP_VARIANT}.common
    {ESP_VARIANT}.{BUILD_TYPE}

Where ``{BUILD_TYPE}`` is ``debug`` or ``release``.

To provide additional settings for a specific variant use the variant prefix such
as ``esp32.debug``, ``esp32c3.release``, ``esp32s2.common``, etc.

etc.
