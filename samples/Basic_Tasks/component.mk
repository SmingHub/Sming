COMPONENT_SOC := esp* host
ARDUINO_LIBRARIES := ArduinoFFT SignalGenerator
ENABLE_TASK_COUNT := 1

ifeq ($(SMING_ARCH),Host)
COMPONENT_SRCDIRS += app/Host
endif
