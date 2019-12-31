#include <FlashString/Array.hpp>

#ifdef ENABLE_OTA_ENCRYPTION
IMPORT_FSTR_ARRAY(OTAUpgrade_DecryptionKey, uint8_t, PROJECT_DIR "/out/OtaUpgrade/decrypt.key.bin");
#endif

#ifdef ENABLE_OTA_SIGNING
IMPORT_FSTR_ARRAY(OTAUpgrade_SignatureVerificationKey, uint8_t, PROJECT_DIR "/out/OtaUpgrade/verify.key.bin");
#endif

DEFINE_FSTR_ARRAY(OTAUpgrade_AppFlashRegionOffsets, uint32_t,
#ifdef RBOOT_SPIFFS_0
    RBOOT_SPIFFS_0,
#endif
#ifdef RBOOT_SPIFFS_1
    RBOOT_SPIFFS_1,
#endif
    0
)
