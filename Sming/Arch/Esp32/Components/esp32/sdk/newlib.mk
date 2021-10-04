#
# newlib
#

# Forces the linker to include heap, syscall, pthread and retargetable locks from this component,
# instead of the implementations provided by newlib.
SDK_UNDEF_SYMBOLS += \
	newlib_include_heap_impl \
	newlib_include_syscalls_impl \
	newlib_include_pthread_impl

# Must link before standard C library
LIBS := newlib $(LIBS)
