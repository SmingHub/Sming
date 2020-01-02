# Future configurability (ATM, there is only signing)

# If enabled, OTA firmware images are digitally signed and the upgrade mechanism will reject
# images with an invalid signature.
# You should not disable this feature unless there is absolutely no way your device is accessed
# trhough a public network.
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
#OTA_ENABLE_ENCRYPTION ?= 0

# If enabled, the build timestamps of the current firmware and the OTA blob are not compared, 
# thus allowing to downgrade to an older firmware version.
# You are strongly advised to keep this option disabled, because otherwise a security fix could 
# simply be reverted by downgrading to an unpatched firmware version.
#OTA_ENABLE_DOWNGRADE ?= 0

COMPONENT_INCDIRS := .

OTATOOL := python $(COMPONENT_PATH)/otatool.py

OTA_CRYPTO_FEATURES =
ifneq ($(OTA_ENABLE_SIGNING),0)
OTA_CRYPTO_FEATURES += Signed
# must be global, because it is used in a public header file
GLOBAL_CFLAGS += -DOTA_SIGNED
endif

ifneq ($(OTA_CRYPTO_FEATURES),)

COMPONENT_DEPENDS := libsodium

RELINK_VARS += OTA_PRIVKEY
OTA_PRIVKEY ?= ota.key

.PHONY: ota-generate-privkey
ota-generate-privkey: ##Generate a new firmware update signing key on explicit request.
	$(Q) $(OTATOOL) genkey --output=$(OTA_PRIVKEY)

$(OTA_PRIVKEY):
	@echo "#############################################################"
	@echo "# Generate new private key for securing OTA upgrade images. #"
	@echo "#                                                           #"
	@echo "# If you wish to reuse a key from another project instead,  #"
	@echo "# run make again with OTA_PRIVKEY pointing to the existing  #"
	@echo "# key file.                                                 #"
	@echo "#############################################################"
	$(Q) $(OTATOOL) genkey --output=$@

# The public key has to be embedded in the application via a generated source file.
# Since every project may use a different key, it cannot be built as part of this component,
# but instead must be build as part of the 'App' component. 
# However, because every component, including 'App', is built in an isolated
# environment, the rules from this Makefile will not be available at 'App's build time.
# To work around this limitation, the public key object file is build in the top level 
# Makefile context (outside of any component) and added directly to COMPONENTS_AR, i.e. 
# the list of archive/object files linked into the final firmware image.
OTA_PUBKEY_SRC := $(abspath $(CMP_App_BUILD_BASE)/OTA_PublicKey.c)
OTA_PUBKEY_OBJ := $(OTA_PUBKEY_SRC:.c=.o)

$(OTA_PUBKEY_SRC): $(OTA_PRIVKEY)
	$(Q) $(OTATOOL) mksource --key=$< --output=$@

$(OTA_PUBKEY_OBJ): $(OTA_PUBKEY_SRC)
	$(vecho) "CC $<"
	$(Q) $(CC) $(addprefix -I,$(INCDIR)) $(CFLAGS) -std=c11 -c $< -o $@

COMPONENTS_AR += $(OTA_PUBKEY_OBJ)

endif # OTA_CRYPTO_FEATURES


# Build final OTA upgrade file 
OTA_UPGRADE_FILE=$(FW_BASE)/firmware.ota

.PHONY: ota-file 
ota-file: $(OTA_UPGRADE_FILE) ## Generate OTA upgrade file (done as part of the default target)

CUSTOM_TARGETS += ota-file

$(OTA_UPGRADE_FILE): $(RBOOT_ROM_0_BIN) $(RBOOT_ROM_1_BIN) $(OTA_PRIVKEY)
	$(Q) $(OTATOOL) mkfile \
		$(if $(OTA_CRYPTO_FEATURES),--key=$(OTA_PRIVKEY)) \
		--rom=$(RBOOT_ROM_0_BIN)@$(RBOOT_ROM0_ADDR) \
		$(if $(RBOOT_ROM_1_BIN),--rom=$(RBOOT_ROM_1_BIN)@$(RBOOT_ROM1_ADDR)) \
		--output=$@

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
