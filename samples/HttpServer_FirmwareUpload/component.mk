SPIFF_FILES = web/
ARDUINO_LIBRARIES := MultipartParser OtaUpgrade

ifndef MAKE_DOCS

web-upload: spiffs-image-update
	$(call WriteFlash,$(SPIFF_START_ADDR)=$(SPIFF_BIN_OUT))

endif
