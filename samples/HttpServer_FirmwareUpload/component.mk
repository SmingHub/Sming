SPIFF_FILES = web/
ARDUINO_LIBRARIES := MultipartParser OtaUpgrade

ifndef MAKE_DOCS

web-upload: spiffs-image-update
	$(call WriteFlash,$(RBOOT_SPIFFS_0)=$(SPIFF_BIN_OUT))

endif
