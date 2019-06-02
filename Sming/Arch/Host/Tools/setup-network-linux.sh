#!/bin/bash
#  Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
#  Created 2015 by Skurydin Alexey
#  http://github.com/SmingHub/Sming
#  All files of the Sming Core are provided under the LGPL v3 license.
#
#  Helper script to setup tap0 networking interface under Linux.
#

if [ $# -eq 1 ]; then
  INTERNET_IF=$1
else
  echo -e "Usage\n\t$0 <internet-network-interface>.\n\nExample:\n\t$0 wlan0";
  exit 1;
fi

# Create a tap0 interface with IP network 192.168.13.1/24
sudo ip tuntap add dev tap0 mode tap user $(whoami)
sudo ip a a dev tap0 192.168.13.1/24
sudo ifconfig tap0 up

# The following lines are needed if you plan to access Internet
sudo sysctl net.ipv4.ip_forward=1
sudo sysctl net.ipv6.conf.default.forwarding=1
sudo sysctl net.ipv6.conf.all.forwarding=1

sudo iptables -t nat -A POSTROUTING -o $INTERNET_IF -j MASQUERADE
sudo iptables -A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
sudo iptables -A FORWARD -i tap0 -o $INTERNET_IF -j ACCEPT

exit 0