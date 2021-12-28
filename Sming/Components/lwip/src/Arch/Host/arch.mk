#
# Host lwip
#

ifeq ($(UNAME),Windows)
	COMPONENT_INCDIRS	+= lwip/contrib/ports/win32/include
	LWIP_ARCH_SRCDIR	:= $(LWIP_ARCH_SRCDIR)/Windows
else
	COMPONENT_INCDIRS	+= lwip/contrib/ports/unix/port/include
	LWIP_ARCH_SRCDIR	:= $(LWIP_ARCH_SRCDIR)/Linux
endif

LWIP_CMAKE_OPTIONS		+= -DHOSTLIB_INCDIR=$(ARCH_COMPONENTS)/hostlib/include
