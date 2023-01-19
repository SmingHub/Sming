cmake_minimum_required(VERSION 3.12)

set(PICO_PLATFORM "rp2040")
set(PICO_COMPILER "pico_arm_gcc")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
# include(pico_extras_import.cmake)

project(pico_lib C CXX ASM)

# Don't want these interfaces
set(SKIP_PICO_MALLOC 1)
set(SKIP_PICO_STDIO 1)
set(SKIP_PICO_STDIO_SEMIHOSTING 1)
set(SKIP_PICO_STDIO_UART 1)
set(SKIP_PICO_STDIO_USB 1)
set(SKIP_TINYUSB 1)

# Initialize the SDK
pico_sdk_init()

add_library(pico STATIC)

# Use a longer XOSC startup time, to accommodate Adafruit and other boards that may need it.
target_compile_definitions(pico PUBLIC
	PICO_PRINTF_ALWAYS_INCLUDED=1
	PICO_FLASH_SIZE_BYTES=16777216
	PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
)

pico_set_program_name(pico "Sming")
pico_set_program_url(pico "https://github.com/SmingHub/Sming")

include_directories(BEFORE ${pico_lib_SOURCE_DIR})

target_link_libraries(pico
	hardware_adc
	hardware_base
	hardware_claim
	hardware_clocks
	hardware_divider
	hardware_dma
	hardware_exception
	hardware_flash
	hardware_gpio
	hardware_i2c
	hardware_interp
	hardware_irq
	hardware_pio
	hardware_pll
	hardware_pwm
	hardware_resets
	hardware_rtc
	hardware_spi
	hardware_sync
	hardware_timer
	hardware_uart
	hardware_vreg
	hardware_watchdog
	hardware_xosc
	pico_bit_ops
	pico_divider
	pico_double
	pico_fix
	pico_float
	pico_int64_ops
	pico_mem_ops
	pico_runtime
	pico_standard_link
	pico_unique_id
	pico_audio_i2s
	pico_cyw43_arch_lwip_poll
)
