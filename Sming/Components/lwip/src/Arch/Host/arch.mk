#
# Host lwip
#

ifeq ($(UNAME),Windows)

	LWIP_ARCH_SRCDIR	:= $(LWIP_ARCH_SRCDIR)/Windows
	NPCAP_SRCDIR		:= $(LWIP_ARCH_SRCDIR)/npcap
	LWIP_CMAKE_OPTIONS	+= -DNPCAP_SRCDIR=$(NPCAP_SRCDIR)
	COMPONENT_INCDIRS	+= \
		lwip/contrib/ports/win32/include \
		$(NPCAP_SRCDIR)/Include
	COMPONENT_PREREQUISITES += $(NPCAP_SRCDIR)/.ok
	PCAP_SRC := npcap-sdk-1.05.zip

$(NPCAP_SRCDIR)/.ok:
	@echo Fetching npcap...
	$(Q) \
		rm -rf $(@D) && \
		mkdir -p $(@D) && \
		cd $(@D) && \
		powershell -Command "Set-Variable ProgressPreference SilentlyContinue; \
			Invoke-WebRequest https://npcap.com/dist/$(PCAP_SRC) -OutFile $(PCAP_SRC); \
			Expand-Archive $(PCAP_SRC) ." && \
		$(call ApplyPatch,$(LWIP_ARCH_SRCDIR)/npcap.patch) && \
		touch $@

else

	COMPONENT_INCDIRS	+= lwip/contrib/ports/unix/port/include
	LWIP_ARCH_SRCDIR	:= $(LWIP_ARCH_SRCDIR)/Linux

endif

COMPONENT_SRCDIRS		+= $(LWIP_ARCH_SRCDIR)
