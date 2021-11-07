COMPONENT_SOC := host
COMPONENT_DEPENDS := OtaUpgradeMqtt
APP_NAME := deployer

SMING_ARCH := Host
ENABLE_SSL := Bearssl

##@Firmware Upgrade

APP=$(TARGET_OUT_0)

pack: application $(APP)  ##Pack firmware files for deployment (HOST_PARAMETERS=inputFile packedFile version 1|0. Example: )
	$(Q) $(APP) pack -- $(HOST_PARAMETERS)
	
deploy: application $(APP) ##Deploy firmware files to MQTT server (HOST_PARAMETERS=packedFile MQTTQ_URL. Example MQTT user: )
	$(Q) $(APP) deploy -- $(HOST_PARAMETERS)
