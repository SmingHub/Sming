PROVIDE ( system_soft_wdt_feed = pp_soft_wdt_feed );
PROVIDE ( system_get_free_heap_size = xPortGetFreeHeapSize );
PROVIDE ( system_os_task = ets_task );
PROVIDE ( system_os_post = ets_post );
PROVIDE ( system_restart = system_restart_local );
PROVIDE ( os_random = phy_get_rand );
PROVIDE ( system_get_cpu_freq = ets_get_cpu_frequency );
