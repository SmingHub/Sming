COMPONENT_SOC := host
DISABLE_NETWORK := 1

# Call the generated executable something different (by default, it's just `app`)
APP_NAME := utility

# We don't need heap monitoring for utility applications
ENABLE_MALLOC_COUNT := 0

#
DISABLE_NETWORK := 1
HOST_NETWORK_OPTIONS := --nonet
