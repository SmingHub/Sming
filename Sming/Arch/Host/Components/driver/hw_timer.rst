Host hw_timer driver
====================

Hardware timers are emulated using a background thread.
The O/S scheduler timeslice granularity is quite coarse and for time-critical threads is
typically 1ms. Therefore, when a timer becomes close to expiry the thread sits ('spins') in a
loop to get better resolution. For the rest of the time the thread is suspended or in a wait state.
