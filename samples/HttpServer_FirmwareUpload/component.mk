SPIFF_FILES = web/
ARDUINO_LIBRARIES := ArduinoJson6 libsodium

# The line below enables the form upload support on the server
ENABLE_HTTP_SERVER_MULTIPART = 1

##@Building

web-pack:
	$(Q) date +'%a, %d %b %Y %H:%M:%S GMT' -u > web/.lastModified

web-upload: web-pack spiffs-image-update
	$(call WriteFlash,$(RBOOT_SPIFFS_0)=$(SPIFF_BIN_OUT))

.PHONY: python-requirements
python-requirements:
	python -m pip install --user -r requirements.txt

SIGNTOOL := python $(COMPONENT_PATH)/signtool.py
SIGNING_KEY := $(COMPONENT_PATH)/signing.key
VERIFICATION_HEADER := $(COMPONENT_PATH)/app/FirmwareVerificationKey.h

$(SIGNING_KEY):
	@echo "#########################################################"
	@echo "# Generating new signing key for firmware update images #"
	@echo "#                                                       #"
	@echo "# If this is not what you want, replace the generated   #"
	@echo "# key without your own (e. g. from a previous project)  #"
	@echo "# and run 'make' again.                                 #"
	@echo "#########################################################"
	$(Q) $(SIGNTOOL) --genkey --keyfile=$@

.PHONY: generate-signing-key
generate-signing-key: ##Generate a new firmware update signing key on explicit request.
	$(Q) $(SIGNTOOL) --genkey --keyfile=$(SIGNING_KEY)

$(VERIFICATION_HEADER): $(SIGNING_KEY)
	$(Q) $(SIGNTOOL) --keyfile=$< --pubkey-header=$@

# add dependency to trigger automatic generation of verification key header
$(COMPONENT_PATH)/app/application.cpp: $(VERIFICATION_HEADER)


# make signed image generation a phony target, because the rboot component and its variables have not been loaded yet
.PHONY: signedrom
signedrom: $(RBOOT_ROM_0_BIN) $(RBOOT_ROM_1_BIN) ##Create signed ROM image (or images if your setup requires two separate ROM images)
	$(info Generating signed firmware update file(s)...)
	$(Q) $(SIGNTOOL) --keyfile=$(SIGNING_KEY) --out "$(RBOOT_ROM_0_BIN).signed" --rom "$(RBOOT_ROM0_ADDR)=$(RBOOT_ROM_0_BIN)"
	$(Q) if [ -n "$(RBOOT_ROM_1_BIN)" ]; then \
		$(SIGNTOOL) --keyfile=$(SIGNING_KEY) --out "$(RBOOT_ROM_1_BIN).signed" --rom "$(RBOOT_ROM1_ADDR)=$(RBOOT_ROM_1_BIN)"; \
	fi


