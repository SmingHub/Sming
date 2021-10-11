#
# pthread
#
SDK_UNDEF_SYMBOLS += \
	pthread_include_pthread_impl \
	pthread_include_pthread_cond_impl \
	pthread_include_pthread_local_storage_impl

ifdef CONFIG_FREERTOS_ENABLE_STATIC_TASK_CLEAN_UP
SDK_WRAP_SYMBOLS += vPortCleanUpTCB
endif
