#include <FlashString/Array.hpp>

namespace OtaUpgrade
{
DEFINE_FSTR_ARRAY(AppFlashRegionOffsets, uint32_t,
#ifdef RBOOT_SPIFFS_0
				  RBOOT_SPIFFS_0,
#endif
#ifdef RBOOT_SPIFFS_1
				  RBOOT_SPIFFS_1,
#endif
				  0)

#ifdef ENABLE_OTA_ENCRYPTION
IMPORT_FSTR_ARRAY(DecryptionKey, uint8_t, PROJECT_DIR "/out/OtaUpgrade/decrypt.key.bin");
#endif

#ifdef ENABLE_OTA_SIGNING
IMPORT_FSTR_ARRAY(SignatureVerificationKey, uint8_t, PROJECT_DIR "/out/OtaUpgrade/verify.key.bin");
#endif

} // namespace OtaUpgrade
