# Uncomment the option below if you want SSL support
#ENABLE_SSL=1

CONFIG_VARS += WS_URL

# Use wss: for secure connection
WS_URL ?= ws://127.0.0.1:8000

COMPONENT_CXXFLAGS += -DWS_URL=\"$(WS_URL)\"
