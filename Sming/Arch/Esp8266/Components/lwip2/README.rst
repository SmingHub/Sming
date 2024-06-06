Esp8266 LWIP Version 2
======================

This Component implements the current Version 2 LWIP stack.
Note that at present espconn\_* functions are not supported.


.. envvar:: TCP_MSS

    Maximum TCP segment size. Default 1460.


.. envvar:: LWIP_IPV6

    default: 0 (disabled)

    Set to enable IPv6 support.


.. envvar:: LWIP_FEATURES

    If anyone knows of an actual reference for this setting, link here please!

    Looking at glue-lwip/arduino/lwipopts.h, setting ``LWIP_FEATURES`` to 1 enables these LWIP flags:

    - IP_FORWARD
    - IP_REASSEMBLY
    - IP_FRAG
    - IP_NAPT (IPV4 only)
    - LWIP_AUTOIP
    - LWIP_DHCP_AUTOIP_COOP
    - LWIP_TCP_SACK_OUT
    - TCP_LISTEN_BACKLOG
    - SNTP_MAX_SERVERS = 3

    Also DHCP discovery gets hooked.
