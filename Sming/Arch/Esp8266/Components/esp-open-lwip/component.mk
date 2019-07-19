COMPONENT_VARS			:= ENABLE_LWIPDEBUG ENABLE_ESPCONN
ENABLE_LWIPDEBUG		?= 0
ENABLE_ESPCONN			?= 0

COMPONENT_DEPENDS		:= esp8266

COMPONENT_CFLAGS := \
	-DPBUF_RSV_FOR_WLAN \
	-DEBUF_LWIP -DLWIP_OPEN_SRC \
	-Wno-implicit-function-declaration \
	-Wno-address \
	-Wno-format \
	-Wno-switch \
	-Wno-pointer-sign \
	-Wno-unused-function

ifeq ($(SMING_RELEASE),1)
	COMPONENT_CFLAGS	+= -DLWIP_NOASSERT
endif

ifeq ($(ENABLE_LWIPDEBUG), 1)
	COMPONENT_CFLAGS	+= -DLWIP_DEBUG
endif

COMPONENT_SUBMODULES	:= esp-open-lwip
COMPONENT_SRCDIRS		:=
COMPONENT_SRCFILES		:= \
	lwip/core/def.c \
	lwip/core/dhcp.c \
	lwip/core/dns.c \
	lwip/core/init.c \
	lwip/core/mem.c \
	lwip/core/memp.c \
	lwip/core/netif.c \
	lwip/core/pbuf.c \
	lwip/core/raw.c \
	lwip/core/sntp.c \
	lwip/core/stats.c \
	lwip/core/sys_arch.c \
	lwip/core/sys.c \
	lwip/core/tcp.c \
	lwip/core/tcp_in.c \
	lwip/core/tcp_out.c \
	lwip/core/timers.c \
	lwip/core/udp.c \
	lwip/core/ipv4/autoip.c \
	lwip/core/ipv4/icmp.c \
	lwip/core/ipv4/igmp.c \
	lwip/core/ipv4/inet.c \
	lwip/core/ipv4/inet_chksum.c \
	lwip/core/ipv4/ip_addr.c \
	lwip/core/ipv4/ip.c \
	lwip/core/ipv4/ip_frag.c \
	lwip/netif/etharp.c \
	\
	lwip/app/dhcpserver.c

ifeq ($(ENABLE_ESPCONN),1)
	COMPONENT_SRCFILES += \
		$(addprefix lwip/app/,\
			espconn.c \
			espconn_tcp.c \
			espconn_udp.c \
			espconn_mdns.c) \
		lwip/api/err.c \
		lwip/core/mdns.c
else
	COMPONENT_SRCFILES	+= espconn_dummy.c
endif
COMPONENT_SRCFILES		:= $(addprefix esp-open-lwip/,$(COMPONENT_SRCFILES))
COMPONENT_INCDIRS		:= esp-open-lwip/include

# Fussy about include paths so override default
INCDIR := \
	$(COMPONENT_PATH)/esp-open-lwip/include \
	$(SDK_BASE)/include \
	$(SMING_HOME)/System/include \
	$(ARCH_SYS)/include \
	$(ARCH_COMPONENTS)/esp8266/include \
	$(SMING_HOME)/Wiring 
