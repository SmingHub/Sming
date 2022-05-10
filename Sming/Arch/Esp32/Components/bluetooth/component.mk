COMPONENT_LIBNAME :=

COMPONENT_DEPENDS := esp32

EXTRA_LIBS		+= \
				bt \
				btdm_app

SDK_LIBDIRS := \
	bt/controller/lib_esp32/$(ESP_VARIANT)/ \
	bt/controller/lib_esp32c3_family/$(ESP_VARIANT)/

LIBDIRS += \
	$(addprefix $(SDK_COMPONENTS_PATH)/,$(SDK_LIBDIRS))

SDK_INCDIRS := \
	bt/include/$(ESP_VARIANT)/include \
	bt/common/api/include/api \
	bt/common/btc/profile/esp/blufi/include  \
	bt/common/btc/profile/esp/include  \
	bt/common/osi/include	\
	bt/host/nimble/nimble/nimble/include                     \
	bt/host/nimble/nimble/nimble/host/include                \
	bt/host/nimble/nimble/porting/nimble/include             \
	bt/host/nimble/nimble/porting/npl/freertos/include       \
	bt/host/nimble/nimble/nimble/host/services/ans/include   \
	bt/host/nimble/nimble/nimble/host/services/bas/include   \
	bt/host/nimble/nimble/nimble/host/services/dis/include   \
	bt/host/nimble/nimble/nimble/host/services/gap/include   \
	bt/host/nimble/nimble/nimble/host/services/gatt/include  \
	bt/host/nimble/nimble/nimble/host/services/ias/include   \
	bt/host/nimble/nimble/nimble/host/services/ipss/include  \
	bt/host/nimble/nimble/nimble/host/services/lls/include   \
	bt/host/nimble/nimble/nimble/host/services/tps/include   \
	bt/host/nimble/nimble/nimble/host/util/include           \
	bt/host/nimble/nimble/nimble/host/store/ram/include      \
	bt/host/nimble/nimble/nimble/host/store/config/include   \
	bt/host/nimble/esp-hci/include                           \
	bt/host/nimble/port/include

COMPONENT_INCDIRS := \
	$(addprefix $(SDK_COMPONENTS_PATH)/,$(SDK_INCDIRS))
	
DISABLE_NETWORK := 0
