ENABLE_SSL		?= 1
ifneq ($(ENABLE_SSL),1)
# Needed for hmac_md5, etc.
	EXTRA_LIBS	:= ssl
endif
