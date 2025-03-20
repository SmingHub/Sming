Dual Core
=========

.. highlight:: text

Basic example of using second core of ESP32, RP2040 and RP2350 chips.

.. important::

    Sming is NOT thread-safe! In general, only re-entrant code and code intended to be called from interrupt context is safe to use from multiple cores.

If code is timing sensitive then it should be run from IRAM. For RP2040 it *must* be in IRAM.

Flash and filing system access must only be done from the main Sming application.


Required steps
--------------

For the RP2040 (and RP2350) Sming builds are 'bare metal' so there is no operating system to consider.

For the ESP32 we require FreeRTOS so the following steps are necessary:

- Override IDF SDK config settings as in ``esp-dual-core.cfg``.
- Add line to project's ``component.mk``: *SDK_CUSTOM_CONFIG := esp32-dual-core.cfg*
- Add application code to run on second core and call to start for that core
- Run `make sdk-config-clean` to ensure custom configuration values are picked up

The custom configuration enables dual-core operation (``CONFIG_FREERTOS_UNICORE=n``) and disables the idle task for the second core (``CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU1=n``). This ensures that code will run uninterrupted.

Task stats shows this when running::

    # | Core | Prio | Handle   | Run Time | % Time | Name
    1 |   0  |   24 | 3ffaee9c |        0 |    0%  | ipc0
    3 |   0  |   22 | 3ffaf568 |     3226 |    0%  | esp_timer
    5 |   0  |    0 | 3ffb8974 |  1996774 |   49%  | IDLE0
    2 |   1  |   24 | 3ffaf404 |        0 |    0%  | ipc1
    8 |   1  |   18 | 3ffbdf44 |    33156 |    0%  | Sming
    7 |   1  |    5 | 3ffb9d24 |  1966844 |   49%  | Sming2
    6 |   1  |    0 | 3ffb9118 |        0 |    0%  | IDLE1


Bare Metal
----------

Because we rely on the IDF and its dependency on FreeRTOS, the above approach also ensures that calls such as ``System.queueCallback`` will work. This is the recommended way to communicate between code running on different cores.

The default mode for Sming runs without FreeRTOS on the second core (*CONFIG_FREERTOS_UNICORE=y*) which provides a smaller set of tasks and thus lower system memory usage::

    # | Core | Prio | Handle   | Run Time | % Time | Name
    1 |   0  |   22 | 3ffaf470 |       29 |    0%  | esp_timer
    4 |   0  |   18 | 3ffb778c |    11016 |    0%  | Sming
    3 |   0  |    0 | 3ffafdb0 |  1988955 |   49%  | IDLE

In this state it is technically possible to get code running on the second core by hooking a low-level startup routine. The application would need to handle stack/heap allocation and code would be far more limited in what it can safely do.
