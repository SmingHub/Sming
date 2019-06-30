EXTRA_LIBS		:= pthread

ifeq ($(UNAME),Windows)
	EXTRA_LIBS	+= wsock32
endif
