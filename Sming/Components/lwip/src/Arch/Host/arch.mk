#
# Host lwip
#

ifeq ($(UNAME),Windows)
	LWIP_ARCH_SRCDIR	:= $(LWIP_ARCH_SRCDIR)/Windows
	COMPONENT_INCDIRS	+= \
		lwip/contrib/ports/win32/include \
		$(LWIP_ARCH_SRCDIR)/src/npcap/Include
else
	COMPONENT_INCDIRS	+= lwip/contrib/ports/unix/port/include
	LWIP_ARCH_SRCDIR	:= $(LWIP_ARCH_SRCDIR)/Linux
endif

COMPONENT_SRCDIRS		+= $(LWIP_ARCH_SRCDIR)
