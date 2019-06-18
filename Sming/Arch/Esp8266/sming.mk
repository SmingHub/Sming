# ESP8266 Sming framework

# Tools

#
# $1 -> target
# $2 -> make directory
# $3 -> parameters
#
define make-tool
	$(Q) mkdir -p $(BUILD_BASE)/$(2)/$(UNAME)
	$(MAKE) --no-print-directory -C $2 TARGET=$(abspath $1) BUILD_DIR=$(abspath $(BUILD_BASE)/$(2)/$(UNAME)) V=$(V) $3
endef
