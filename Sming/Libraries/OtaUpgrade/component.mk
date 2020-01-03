COMPONENT_SRCDIRS :=
COMPONENT_SRCFILES := OtaUpgrade/BasicStream.cpp
COMPONENT_APPCODE := appcode

COMPONENT_INCDIRS := .

COMPONENT_DOXYGEN_INPUT := .

OTATOOL := python $(COMPONENT_PATH)/otatool.py

# Create a directory for generated source code (keys, build timestamp)
OTA_GENCODE_DIR := out/OtaUpgrade
$(OTA_GENCODE_DIR):
	$(Q) mkdir -p $@
clean: ota-gencode-clean
.PHONY: ota-gencode-clean
ota-gencode-clean:
	-$(Q) rm -rf $(OTA_GENCODE_DIR)
# Tell build system about it so any known source files will be compiled and linked
COMPONENT_APPCODE += $(abspath $(OTA_GENCODE_DIR))


COMPONENT_VARS += ENABLE_OTA_SIGNING 
ENABLE_OTA_SIGNING ?= 1

ifeq ($(ENABLE_OTA_SIGNING),1)
OTA_CRYPTO_FEATURES += --signed
# has to be global, because it is used in a public header file
GLOBAL_CFLAGS += -DENABLE_OTA_SIGNING
endif

COMPONENT_VARS += ENABLE_OTA_ENCRYPTION
ENABLE_OTA_ENCRYPTION ?= 0

ifeq ($(ENABLE_OTA_ENCRYPTION),1)
OTA_CRYPTO_FEATURES += --encrypted
GLOBAL_CFLAGS += -DENABLE_OTA_ENCRYPTION
COMPONENT_SRCFILES += OtaUpgrade/EncryptedStream.cpp
endif

ifneq ($(OTA_CRYPTO_FEATURES),)

COMPONENT_DEPENDS := libsodium

RELINK_VARS += OTA_KEY
OTA_KEY ?= ota.key

.PHONY: ota-genkey
ota-genkey: ##Generate a new firmware update signing key on explicit request.
	$(Q) $(OTATOOL) genkey --output=$(OTA_KEY)

$(OTA_KEY):
	@echo "################################################################"
	@echo "# Generate new private key for securing OTA upgrade images.    #"
	@echo "#                                                              #"
	@echo "# If you wish to reuse a key from another project instead, run #"
	@echo "# make again with OTA_KEY pointing to the existing key file.   #"
	@echo "################################################################"
	$(Q) $(OTATOOL) genkey --output=$@

# Generate public keys from the selected private key, output as binary files
# These are pulled as appcode using IMPORT_FSTR
OTA_DECRYPT_KEY_BIN := $(OTA_GENCODE_DIR)/decrypt.key.bin
OTA_VERIFY_KEY_BIN := $(OTA_GENCODE_DIR)/verify.key.bin

App-build: $(OTA_DECRYPT_KEY_BIN) $(OTA_VERIFY_KEY_BIN)
$(OTA_DECRYPT_KEY_BIN) $(OTA_VERIFY_KEY_BIN): $(OTA_KEY) | $(OTA_GENCODE_DIR)
	$(Q) $(OTATOOL) mkbinkeys --key=$< --output=$(OTA_GENCODE_DIR)

endif # OTA_CRYPTO_FEATURES

# Downgrade protection:
COMPONENT_VARS += ENABLE_OTA_DOWNGRADE
ENABLE_OTA_DOWNGRADE ?= 0

ifeq ($(ENABLE_OTA_DOWNGRADE),1)
COMPONENT_CXXFLAGS += -DENABLE_OTA_DOWNGRADE
else
OTA_BUILD_TIMESTAMP_SRC := $(OTA_GENCODE_DIR)/BuildTimestamp.cpp

# Date reference for build timestamps, generated using this:
# date --date 1900-01-01 +%s%3NLL
# Note: Out of range in bash 3.1.23 (MinGW) hence included here as a constant
OTA_DATE_REF := -2208988800000LL

# Using a phony target, the source file containing the build timestamp is regenerated with every build.
# (This also enforces relinking the firmware with every make invocation, even if nothing has changed, but Sming does that anyway.)
.PHONY: _ota-make-build-timestamp
_ota-make-build-timestamp: | $(OTA_GENCODE_DIR)
	$(Q) echo '#include <FakePgmSpace.h>' > $(OTA_BUILD_TIMESTAMP_SRC)
	$(Q) echo 'namespace OtaUpgrade {' >> $(OTA_BUILD_TIMESTAMP_SRC)
	$(Q) echo 'extern const uint64_t BuildTimestamp PROGMEM = $(shell date +%s%3NLL) - $(OTA_DATE_REF);' >> $(OTA_BUILD_TIMESTAMP_SRC)
	$(Q) echo '} // namespace OtaUpgrade' >> $(OTA_BUILD_TIMESTAMP_SRC)

App-build: _ota-make-build-timestamp

endif


OTA_CRYPTO_FEATURES_IMAGE := $(OTA_CRYPTO_FEATURES)
OTA_KEY_IMAGE := $(OTA_KEY)
-include ota-rollover.mk

.PHONY: ota-rollover, ota-rollover-done
ifndef OTA_ROLLOVER_IN_PROGRESS
ota-rollover:
	$(if $(OTA_CRYPTO_FEATURES),$(Q) cp $(OTA_KEY) ota-rollover.key)
	$(Q) echo '# Automatically generated file. Do not edit.' > ota-rollover.mk
	$(Q) echo 'OTA_ROLLOVER_IN_PROGRESS := 1' >> ota-rollover.mk
	$(Q) echo 'OTA_KEY_IMAGE := ota-rollover.key' >> ota-rollover.mk
	$(Q) echo 'OTA_CRYPTO_FEATURES_IMAGE := $(OTA_CRYPTO_FEATURES)' >> ota-rollover.mk
	@echo
	@echo "===== OTA upgrade key/setting rollover now in progress ====="
	@echo
	@echo "You may now change the OTA security settings and/or generate"
	@echo "a new key using 'make ota-generate-privkey'."
	@echo "Afterwards, run 'make' to generate the rollover image."
	@echo
ota-rollover-done:
	$(error No OTA rollover in progress!. Run 'make ota-rollover' to start the rollover process)
else
ota-rollover:
	$(error OTA rollover already in progress! Run 'make ota-rollover-done' to complete the rollover process)

ota-rollover-done:
	$(if $(OTA_CRYPTO_FEATURES_IMAGE),$(Q) rm ota-rollover.key)
	$(Q) rm ota-rollover.mk
	@echo
	@echo "===== OTA upgrade key/setting rollover completed ====="
	@echo
endif


# Build final OTA upgrade file
OTA_UPGRADE_FILE = $(FW_BASE)/firmware.ota

.PHONY: ota-file 
ota-file: $(OTA_UPGRADE_FILE) ##Generate OTA upgrade file (done as part of the default target)

ifneq ($(SMING_ARCH),Host)
CUSTOM_TARGETS += ota-file
endif

$(OTA_UPGRADE_FILE): $(RBOOT_ROM_0_BIN) $(RBOOT_ROM_1_BIN) $(OTA_KEY_IMAGE)
ifeq ($(ENABLE_OTA_DOWNGRADE),0)
ifeq ($(OTA_CRYPTO_FEATURES),)
	$(warning WARNING: Downgrade protection ineffective without encryption or digital signature. \
		Consider setting ENABLE_OTA_SIGNING or ENABLE_OTA_ENCRYPTION to 1!)
endif
endif
	$(Q) $(OTATOOL) mkfile \
		$(OTA_CRYPTO_FEATURES_IMAGE) \
		$(if $(OTA_CRYPTO_FEATURES_IMAGE),--key=$(OTA_KEY_IMAGE)) \
		--rom=$(RBOOT_ROM_0_BIN)@$(RBOOT_ROM0_ADDR) \
		$(if $(RBOOT_ROM_1_BIN),--rom=$(RBOOT_ROM_1_BIN)@$(RBOOT_ROM1_ADDR)) \
		--output=$@
ifdef OTA_ROLLOVER_IN_PROGRESS
	@echo
	@echo "===== OTA upgrade rollover image created ====="
	@echo
	@echo "After you have upgraded your device(s) using the rollover image"
	@echo "run 'make ota-rollover-done' to complete the rollover process."
	@echo
endif

# Apply sanity checks to image security settings
define _ota-verify-boolean-setting
ifneq ($$(filter-out 1 0,$($1)),)
$$(error $1 has invalid value '$($1)'. Use '0' or '1' instead)
endif
endef
$(eval $(call _ota-verify-boolean-setting,ENABLE_OTA_SIGNING))
$(eval $(call _ota-verify-boolean-setting,ENABLE_OTA_ENCRYPTION))
$(eval $(call _ota-verify-boolean-setting,ENABLE_OTA_DOWNGRADE))

# Convenience target for uploading file via HTTP POST
CACHE_VARS += OTA_UPLOAD_URL OTA_UPLOAD_NAME
OTA_UPLOAD_URL ?= 
OTA_UPLOAD_NAME ?= firmware

# otatool.py includes a HTTP POST upload feature, no need to use install additional tools like curl
.PHONY: ota-upload
ota-upload: $(OTA_UPGRADE_FILE) ##Perform OTA upgrade via HTTP POST file upload (set OTA_UPLOAD_URL first!)
	$(Q) if [ -n "$(OTA_UPLOAD_URL)" ]; then \
		$(OTATOOL) upload --field=$(OTA_UPLOAD_NAME) --url=$(OTA_UPLOAD_URL) $(OTA_UPGRADE_FILE); \
	else echo Please set OTA_UPLOAD_URL to use this target.; \
	fi
