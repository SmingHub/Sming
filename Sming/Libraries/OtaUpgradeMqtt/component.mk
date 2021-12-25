COMPONENT_SRCDIRS  := 
COMPONENT_SRCFILES := src/PayloadParser.cpp src/StandardPayloadParser.cpp 
COMPONENT_INCDIRS  := src/include
COMPONENT_DOXYGEN_INPUT := src/include

# If enabled (set to 1) then we can use all sophisticated mechanisms to upgrade the firmware using the ``OtaUpgrade`` library.
COMPONENT_VARS := ENABLE_OTA_ADVANCED
ENABLE_OTA_ADVANCED ?= 0

COMPONENT_DEPENDS := Ota
ifneq ($(ENABLE_OTA_ADVANCED),0)
	COMPONENT_SRCFILES += src/AdvancedPayloadParser.cpp
	COMPONENT_DEPENDS  += OtaUpgrade
endif

# If enabled (set to 1) then we can use unlimited number of patch versions
COMPONENT_VARS += ENABLE_OTA_VARINT_VERSION 
ENABLE_OTA_VARINT_VERSION ?= 1

COMPONENT_CXXFLAGS := -DENABLE_OTA_ADVANCED=$(ENABLE_OTA_ADVANCED) \
					  -DENABLE_OTA_VARINT_VERSION=$(ENABLE_OTA_VARINT_VERSION)
				
##@Firmware Upgrade
					  
OTA_TOOLS := $(COMPONENT_PATH)/tools
OTA_DEPLOYMENT_TOOL = $(OTA_TOOLS)/deployer/out/Host/debug/firmware/deployer$(TOOL_EXT)

$(OTA_DEPLOYMENT_TOOL):
	$(Q) $(MAKE) -C $(OTA_TOOLS)/deployer SMING_ARCH=Host ENABLE_CUSTOM_LWIP=2


# SDP = Sming Deployment Package
OTA_PACKAGE_EXT =.sdp

OTA_PATCH_VERSION ?= $(shell date +%s)

PACKAGE_IN = $(RBOOT_ROM_0_BIN)
ifneq ($(ENABLE_OTA_ADVANCED), 0)
	PACKAGE_IN = $(OTA_UPGRADE_FILE)
endif
PACKAGE_OUT = $(PACKAGE_IN)$(OTA_PACKAGE_EXT)

.PHONY: ota-pack
ota-pack: $(PACKAGE_OUT) ##Creates a deployment package from the current application (use OTA_PATCH_VERSION to specify the version number)

$(PACKAGE_OUT): $(PACKAGE_IN) $(OTA_DEPLOYMENT_TOOL) 
	$(Q) $(OTA_DEPLOYMENT_TOOL) pack --debug=0 --nonet -- $(PACKAGE_IN) $(PACKAGE_OUT) $(OTA_PATCH_VERSION) $(ENABLE_OTA_VARINT_VERSION)

.PHONY: ota-deploy
ota-deploy: $(PACKAGE_OUT) ##Uploads new firmware version of the current application (use MQTT_FIRMWARE_URL to specify the MQTT URL)
	$(Q) $(OTA_DEPLOYMENT_TOOL) deploy --debug=0 -- $(PACKAGE_OUT) $(MQTT_FIRMWARE_URL)
