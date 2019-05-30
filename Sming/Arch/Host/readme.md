# Sming Host Emulator

## Summary

Allows Sming applications to be built as an executable to run on the development host (Windows or Linux).

This is a source-level emulator for developing and testing new framework code prior to flashing a real device.

This is not a machine emulator; if you need something operating at a lower level take a look at [QEMU](https://www.qemu.org/).

## Requirements

`CMake` is required to build LWIP

Ensure you are using relatively recent compilers, with 32-bit libraries available.

For Linux, you may require the `gcc-multilib` and `g++-multilib` packages to build 32-bit executables on a 64-bit OS.

For Windows, make sure your `MinGW` distro is up to date. If you run `gcc --version` you should get `gcc (MinGW.org GCC-6.3.0-1) 6.3.0` or later. If it's older, execute these commands:

```
mingw-get update
mingw-get upgrade
```

## Building

Build the framework and application as usual, specifying `SMING_ARCH=Host`. For example:

```
cd $SMING_HOME
make SMING_ARCH=Host
cd $SMING_HOME/../samples/Basic_Serial
make SMING_ARCH=Host
```

This builds the application as an executable in `out/firmware/app`.
Various command-line options are supported, use `--help` for details.

Use `make run` to execute the application. Command-line parameters are passed in `SMING_TARGET_OPTIONS` so you can customise this via environment or application makefile thus:

`export SMING_TARGET_OPTIONS="--pause --uart=0"`

To find out what options are in force, use `make list-config`.

## Features

### Flash memory

This is emulated using a backing file. By default, it's in `flash.bin` in the current directory.

Use `make flashinit` to clear and reset the file.
Use `make flashfs` to copy the generated SPIFFS image into the backing file. `make flash` does the same then runs the application.
Use `make flash` to do a `flashfs` then a `run`

### UART (serial) ports

Multiple serial terminals are supported via raw TCP network sockets.

For example, start the `Basic_Serial` sample application we build above, with support for both UARTs using the following options:

`out/firmware/app --pause --uart=0 --uart=1`

Note: if you don't specify the `pause` option then the Sming application will start running immediately and any serial output will be discarded.

In separate command windows, start two telnet sessions (a terminal for each serial port):

```
telnet localhost 10000
telnet localhost 10001
```

In the application window, press Enter.

Note: For Windows users, `putty` is generally a better choice. For example, you can configure it to implicitly perform carriage-return for linefeed (i.e. "\n" -> "\r\n"). Run using:

```
putty telnet://localhost:10000
```

Port numbers are allocated sequentially from 10000. If you want to use different port numbers, use `--uartport` option.

### Digital I/O

At present the emulator just writes output to the console. Inputs all return 0.

### Network

#### Linux

Support is provided via TAP network interface (a virtual network layer operating at the ethernet frame level). A TAP interface must be created first, and requires root priviledge:

	sudo ip tuntap add dev tap0 mode tap user `whoami`
	sudo ip a a dev tap0 192.168.13.1/24
	sudo ifconfig tap0 up

This creates the `tap0` interface. The emulator will automatically select the first `tap` interface found. To override this, use the `--ifname` option. An IP address will be assigned, but can be changed using the `--ipaddr` option.

If your application needs to access the internet, additional setup is required:

	sudo sysctl net.ipv4.ip_forward=1
	sudo sysctl net.ipv6.conf.default.forwarding=1
	sudo sysctl net.ipv6.conf.all.forwarding=1
	
	export INTERNET_IF=wlan0 # <!--- Make sure to replace wlan0 with the network interface connected to Internet
	
	sudo iptables -t nat -A POSTROUTING -o $INTERNET_IF -j MASQUERADE
	sudo iptables -A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
	sudo iptables -A FORWARD -i tap0 -o $INTERNET_IF -j ACCEPT


#### Windows

Requires [NPCAP](https://nmap.org/npcap/) library to be installed. Provided with current (3.0.2) version of [Wireshark](https://www.wireshark.org/download.html).

By default, the first valid network adapter will be used, with address assigned via DHCP.

If the adapter is wrong, get a list thus:

	out\firmware\app --ifname=?

	...

	Available adapters:
	- 0: {ACC6BFB2-A15B-4CF8-B93A-8D97644D0AAC} - Oracle
	        192.168.56.1 / 255.255.255.0
	- 1: {A12D4DD0-0EA8-435D-985E-A1F96F781EF0} - NdisWan Adapter
	- 2: {3D66A354-39DD-4C6A-B9C4-14EE223FC3D1} - MS NDIS 6.0 LoopBack Driver
	        0.0.0.0 / 255.0.0.0
	- 3: {BC53D919-339E-4D70-8573-9D7A8AE303C7} - NdisWan Adapter
	- 4: {3CFD43EA-9CC7-44A7-83D4-EB04DD029FE7} - NdisWan Adapter
	- 5: {530640FF-A9C3-436B-9EA2-65102C788119} - Realtek PCIe GBE Family Controller
	        192.168.1.70 / 255.255.255.0
	- 6: {0F649280-BAC2-4515-9CE3-F7DFBB6A1BF8} - Kaspersky Security Data Escort Adapter
	        10.102.37.150 / 255.255.255.252

Then use the appropriate number:

	out\firmware\app --ifname=5

You can also specify by GUID, or a part of it:

	out\firmware\app --ifname={530640FF

The network adapter may be autodetected from a provided ip address, but you must also give the network gateway must also be provided. For example:

	out\firmware\app --ipaddr=192.168.1.10 --gateway=192.168.1.254

You can find gateway addresses using the `ipconfig` command.

We can then run using

`out/firmware/app --ifname=4 --gateway=192.168.1.254 --ipaddr=192.168.1.10`

To use these settings for a `make run`, do this:

`set SMING_TARGET_OPTIONS="--ifname=4 --gateway=192.168.1.254 --ipaddr=192.168.1.10"`

## todo

Add passthrough support for real serial ports to permit connection of physical devices.

Consider how this mechanism might be used to support emulation of other devices (SPI, I2C, etc).

Development platforms with SPI or I2C (e.g. Raspberry Pi) could be supported.

Are there any generic device emulators available? For example, to simulate specific types of SPI slave.

All code is intended to run on either Windows (MinGW) or Linux as simply as possible, without requiring any additional dependencies. If things get more complicated then we might need to consider using external libraries, such as Boost.
