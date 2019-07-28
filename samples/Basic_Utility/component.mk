
# Call the generated executable something different (by default, it's just `app`)
APP_NAME := utility

# We don't need heap monitoring for utility applications
ENABLE_MALLOC_COUNT := 0

# Don't attempt to build any filesystem images
DISABLE_SPIFFS := 1

#
HOST_NETWORK_OPTIONS := --nonet
