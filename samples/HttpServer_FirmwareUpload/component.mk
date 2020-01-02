SPIFF_FILES = web/
ARDUINO_LIBRARIES := libsodium MultipartParser
COMPONENT_DEPENDS := OtaUpgrade

web-upload: spiffs-image-update
	$(call WriteFlash,$(RBOOT_SPIFFS_0)=$(SPIFF_BIN_OUT))

.PHONY: python-requirements
python-requirements:
	$(PYTHON) -m pip install --user -r requirements.txt
