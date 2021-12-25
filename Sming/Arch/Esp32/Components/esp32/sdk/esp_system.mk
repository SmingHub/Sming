#
# esp_system
#

# After system initialization, `start_app` (and its other cores variant) is called.
# This is provided by the user or from another component. Since we can't establish
# dependency on what we don't know, force linker to not drop the symbol regardless
# of link line order.
SDK_UNDEF_SYMBOLS += start_app

ifndef CONFIG_ESP_SYSTEM_SINGLE_CORE_MODE
SDK_UNDEF_SYMBOLS += start_app_other_cores
endif

# ld_include_panic_highint_hdl is added as an undefined symbol because otherwise the
# linker will ignore panic_highint_hdl.S as it has no other files depending on any
# symbols in it.
SDK_UNDEF_SYMBOLS += ld_include_panic_highint_hdl
