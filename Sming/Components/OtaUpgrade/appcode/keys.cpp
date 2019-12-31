#include <FlashString/Array.hpp>

#ifdef OTA_ENCRYPTED
IMPORT_FSTR_ARRAY(OTAUpgrade_EncryptionKey, uint8_t, PROJECT_DIR "/out/OtaUpgrade/encrypt.key.bin");
#endif

#ifdef OTA_SIGNED
IMPORT_FSTR_ARRAY(OTAUpgrade_SignatureVerificationKey, uint8_t, PROJECT_DIR "/out/OtaUpgrade/signing.key.bin");
#endif
