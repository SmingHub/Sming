#
# app_update
#

# esp_app_desc structure is added as an undefined symbol because otherwise the
# linker will ignore this structure as it has no other files depending on it.
SDK_UNDEF_SYMBOLS += esp_app_desc
