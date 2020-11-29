COMPONENT_DEPENDS := \
	arch_driver \
	esp32

COMPONENT_SRCDIRS			:= .
COMPONENT_INCDIRS			:= include

COMPONENT_DOXYGEN_INPUT		:= include/driver

# hw_timer
COMPONENT_VARS				+= USE_US_TIMER
USE_US_TIMER				?= 1
ifeq ($(USE_US_TIMER),1)
GLOBAL_CFLAGS				+= -DUSE_US_TIMER
endif
