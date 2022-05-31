ifeq ($(SMING_ARCH),Esp32)
HWCONFIG := ftpserver-esp32
else
HWCONFIG := ftpserver-esp8266
endif
SPIFF_FILES :=
