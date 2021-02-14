Spiffsgen
=========

Test SPIFFS images used by `Spiffs` module.
Content is from `HttpServer_ConfigNetwork` sample, `web/build` directory.

spiff_rom_orig.bin
    Built using SPIFFY with no metadata
spiff_rom_meta.bin
    Build using SPIFFY with metadata (16 bytes)

This test application builds an image using spiffsgen for comparison.

Will fail if built with SPIFFS_OBJ_META_LEN set to anything other than 0 or 16.
