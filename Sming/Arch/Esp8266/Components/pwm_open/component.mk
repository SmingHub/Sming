# ESP8266_new_pwm

# => SMING

CONFIG_VARS += ENABLE_CUSTOM_PWM
ENABLE_CUSTOM_PWM	?= 1
ifeq ($(ENABLE_CUSTOM_PWM), 1)
	PWM_BASE		:= $(ARCH_COMPONENTS)/pwm/new-pwm
	SUBMODULES		+= $(PWM_BASE)
	CFLAGS			+= -DSDK_PWM_PERIOD_COMPAT_MODE=1
	LIBPWM			:= pwm_open
	LIBS			+= $(LIBPWM)
	CLEAN			+= pwm-clean

$(call UserLibPath,$(LIBPWM)): | $(PWM_BASE)/.submodule
	$(Q) $(CC) $(INCDIR) $(CFLAGS) -c $(PWM_BASE)/pwm.c -o $(PWM_BASE)/pwm.o
	$(Q) $(AR) rcs $@ $(PWM_BASE)/pwm.o

.PHONY: pwm-clean
pwm-clean:
	-$(Q) rm -f $(PWM_BASE)/*.o
endif



# => APP

# => PWM
CONFIG_VARS += ENABLE_CUSTOM_PWM
ENABLE_CUSTOM_PWM ?= 1
ifeq ($(ENABLE_CUSTOM_PWM), 1)
	LIBPWM			:= pwm_open
	LIBPWM_DST		:= $(call UserLibPath,$(LIBPWM))
	CUSTOM_TARGETS	+= $(LIBPWM_DST)

$(LIBPWM_DST):
	$(call MakeLibrary,$@,ENABLE_CUSTOM_PWM=1)
else
	LIBPWM := pwm
endif
LIBS += $(LIBPWM)
