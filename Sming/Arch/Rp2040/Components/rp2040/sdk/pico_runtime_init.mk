# pico_runtime_init

RUNTIME_INIT_FUNC += \
	runtime_init_bootrom_reset \
	runtime_init_per_core_bootrom_reset \
	runtime_init_per_core_h3_irq_registers \
	runtime_init_early_resets \
	runtime_init_usb_power_down \
	runtime_init_per_core_enable_coprocessors \
	runtime_init_clocks \
	runtime_init_post_clock_resets \
	runtime_init_rp2040_gpio_ie_disable \
	runtime_init_spin_locks_reset \
	runtime_init_install_ram_vector_table
