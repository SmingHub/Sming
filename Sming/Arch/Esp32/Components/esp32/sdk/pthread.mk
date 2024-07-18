#
# pthread
#
SDK_UNDEF_SYMBOLS += \
	pthread_include_pthread_impl \
	pthread_include_pthread_cond_impl \
	pthread_include_pthread_cond_var_impl \
	pthread_include_pthread_local_storage_impl \
	pthread_include_pthread_rwlock_impl \
	pthread_include_pthread_semaphore_impl

ifdef CONFIG_FREERTOS_ENABLE_STATIC_TASK_CLEAN_UP
SDK_WRAP_SYMBOLS += vPortCleanUpTCB
endif
