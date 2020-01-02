# Future configurability (ATM, there is only signing)

# If enabled, OTA firmware images are digitally signed and the upgrade mechanism will reject
# images with an invalid signature.
# You should not disable this feature unless there is absolutely no way your device is accessed
# through a public network.
COMPONENT_VARS += OTA_ENABLE_SIGNING
OTA_ENABLE_SIGNING ?= 1

# If enabled, OTA firmware images are encrypted.
# It is strongly encouraged to enable this feature if your firmware images contain secret information
# (WiFi credentials, server certificates, etc.)
# However, keep in mind the inherent weekness of this approach: An attacker with physical access to your device
# can always extract the secret information, INCLUDING the private decryption key, which breaks all encryptions
# using the same key (i.e. future firmware images of this project, other projects using the same key...)
# Due to this weakness, encryption is disabled by default. You should think twice if this meachnism provides
# the necessary security for your application or if another method (e.g. TLS) is more appropriate.
COMPONENT_VARS += OTA_ENABLE_ENCRYPTION
OTA_ENABLE_ENCRYPTION ?= 0

# If enabled, the build timestamps of the current firmware and the OTA blob are not compared, 
# thus allowing to downgrade to an older firmware version.
# You are strongly advised to keep this option disabled, because otherwise a security fix could 
# simply be reverted by downgrading to an unpatched firmware version.
COMPONENT_VARS += OTA_ENABLE_DOWNGRADE
OTA_ENABLE_DOWNGRADE ?= 0

COMPONENT_INCDIRS := .

OTATOOL := python $(COMPONENT_PATH)/otatool.py

ifneq ($(OTA_ENABLE_SIGNING),0)
OTA_CRYPTO_FEATURES += --signed
# has to be global, because it is used in a public header file
GLOBAL_CFLAGS += -DOTA_SIGNED
endif
ifneq ($(OTA_ENABLE_ENCRYPTION),0)
OTA_CRYPTO_FEATURES += --encrypted
GLOBAL_CFLAGS += -DOTA_ENCRYPTED
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

# The public key has to be embedded in the application via a generated source file.
# Since every project may use a different key, it cannot be built as part of this component,
# but instead must be build as part of the 'App' component. 
# However, because every component, including 'App', is built in an isolated
# environment, the rules from this Makefile will not be available at 'App's build time.
# To work around this limitation, the public key object file is build in the top level 
# Makefile context (outside of any component) and added directly to COMPONENTS_AR, i.e. 
# the list of archive/object files linked into the final firmware image.
OTA_KEYS_SRC := $(abspath $(CMP_App_BUILD_BASE)/OTA_Keys.c)
OTA_KEYS_OBJ := $(OTA_KEYS_SRC:.c=.o)

# Trigger regeneration of OTA_KEYS_SRC on change of security settings.
# Otherwise a secret key might leak into an unencrypted OTA file when switching from encrypted to unencrypted mode!
OTA_CURRENT_SETTINGS_FILE := $(abspath $(CMP_App_BUILD_BASE)/ota-security.mk)
-include $(OTA_CURRENT_SETTINGS_FILE)
ifneq ($(OTA_CRYPTO_FEATURES_CURRENT),$(OTA_CRYPTO_FEATURES))
.PHONY: $(OTA_KEYS_SRC)
endif

$(OTA_KEYS_SRC): $(OTA_KEY)
	$(Q) $(OTATOOL) mksource --key=$< --output=$@ $(OTA_CRYPTO_FEATURES)
	$(Q) echo 'OTA_CRYPTO_FEATURES_CURRENT = $(OTA_CRYPTO_FEATURES)' > $(OTA_CURRENT_SETTINGS_FILE)

$(OTA_KEYS_OBJ): $(OTA_KEYS_SRC)
	$(vecho) "CC $<"
	$(Q) $(CC) $(addprefix -I,$(INCDIR)) $(CFLAGS) -std=c11 -c $< -o $@

COMPONENTS_AR += $(OTA_KEYS_OBJ)

endif # OTA_CRYPTO_FEATURES

# Downgrade protection:
ifneq ($(OTA_ENABLE_DOWNGRADE),1)
COMPONENT_CFLAGS += -DOTA_DOWNGRADE_PROTECTION

OTA_BUILD_TIMESTAMP_SRC := $(abspath $(CMP_App_BUILD_BASE)/OTA_BuildTimestamp.c)
OTA_BUILD_TIMESTAMP_OBJ := $(OTA_BUILD_TIMESTAMP_SRC:.c=.o)

# Using a phony target, the source file containing the build timestamp is regenerated with every build.
# (This also enforces relinking the firmware with every make invocation, even if nothing has changed, but Sming does that anyway.)
.PHONY: _ota-make-build-timestamp
_ota-make-build-timestamp:
	$(Q) echo '#include <FakePgmSpace.h>' > $(OTA_BUILD_TIMESTAMP_SRC)
	$(Q) echo 'const unsigned long long OTA_BuildTimestamp PROGMEM = $(shell date +%s%3NLL) - $(shell date --date 1900-01-01 +%s%3NLL);' >> $(OTA_BUILD_TIMESTAMP_SRC)

$(OTA_BUILD_TIMESTAMP_OBJ): _ota-make-build-timestamp
	$(vecho) "CC $(OTA_BUILD_TIMESTAMP_SRC)"
	$(Q) $(CC) $(addprefix -I,$(INCDIR)) $(CFLAGS) -std=c11 -c $(OTA_BUILD_TIMESTAMP_SRC) -o $@

COMPONENTS_AR += $(OTA_BUILD_TIMESTAMP_OBJ)
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
OTA_UPGRADE_FILE=$(FW_BASE)/firmware.ota

.PHONY: ota-file 
ota-file: $(OTA_UPGRADE_FILE) ## Generate OTA upgrade file (done as part of the default target)

CUSTOM_TARGETS += ota-file

$(OTA_UPGRADE_FILE): $(RBOOT_ROM_0_BIN) $(RBOOT_ROM_1_BIN) $(OTA_KEY_IMAGE)
ifneq ($(OTA_ENABLE_DOWNGRADE),1)
ifeq ($(OTA_CRYPTO_FEATURES),)
	$(warning WARNING: Downgrade protection ineffective without encryption or digital signature. Consider setting OTA_ENABLE_SIGNING or OTA_ENABLE_ENCRYPTION to 1!)
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


# Convenience target for uploading file via HTTP POST
CACHE_VARS += OTA_UPLOAD_URL OTA_UPLOAD_NAME
OTA_UPLOAD_URL ?= 
OTA_UPLOAD_NAME ?= firmware

# otatool.py includes a HTTP POST upload feature, no need to use install additional tools like curl
.PHONY: ota-upload
ota-upload: $(OTA_UPGRADE_FILE) ## Perform OTA upgrade via HTTP POST file upload (set OTA_UPLOAD_URL first!)
	$(Q) if [ -n "$(OTA_UPLOAD_URL)" ]; then \
		$(OTATOOL) upload --field=$(OTA_UPLOAD_NAME) --url=$(OTA_UPLOAD_URL) $(OTA_UPGRADE_FILE); \
	else echo Please set OTA_UPLOAD_URL to use this target.; \
	fi
