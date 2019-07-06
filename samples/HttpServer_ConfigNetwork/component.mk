SPIFF_FILES = web/build
ARDUINO_LIBRARIES := ArduinoJson6

web-pack:
	$(Q) gulp
	$(Q) date +'%a, %d %b %Y %H:%M:%S GMT' -u > web/build/.lastModified
	
web-upload: web-pack spiff_update
	$(call WriteFlash,$(SPIFF_START_OFFSET)=$(SPIFF_BIN_OUT))
