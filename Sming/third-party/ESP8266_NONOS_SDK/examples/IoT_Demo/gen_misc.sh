#!/bin/bash

echo "gen_misc.sh version 20150511"
echo ""

echo "Please follow below steps(1-5) to generate specific bin(s):"
echo "STEP 1: choose boot version(0=boot_v1.1, 1=boot_v1.2+, 2=none)"
echo "enter(0/1/2, default 2):"
read input

if [ -z "$input" ]; then
    boot=none
elif [ $input == 0 ]; then
	boot=old
elif [ $input == 1 ]; then
    boot=new
else
    boot=none
fi

echo "boot mode: $boot"
echo ""

echo "STEP 2: choose bin generate(0=eagle.flash.bin+eagle.irom0text.bin, 1=user1.bin, 2=user2.bin)"
echo "enter (0/1/2, default 0):"
read input

if [ -z "$input" ]; then
    if [ $boot != none ]; then
    	boot=none
	echo "ignore boot"
    fi
    app=0
    echo "generate bin: eagle.flash.bin+eagle.irom0text.bin"
elif [ $input == 1 ]; then
    if [ $boot == none ]; then
    	app=0
	echo "choose no boot before"
	echo "generate bin: eagle.flash.bin+eagle.irom0text.bin"
    else
	app=1
        echo "generate bin: user1.bin"
    fi
elif [ $input == 2 ]; then
    if [ $boot == none ]; then
    	app=0
	echo "choose no boot before"
	echo "generate bin: eagle.flash.bin+eagle.irom0text.bin"
    else
    	app=2
    	echo "generate bin: user2.bin"
    fi
else
    if [ $boot != none ]; then
    	boot=none
	echo "ignore boot"
    fi
    app=0
    echo "generate bin: eagle.flash.bin+eagle.irom0text.bin"
fi

echo ""

echo "STEP 3: choose spi speed(0=20MHz, 1=26.7MHz, 2=40MHz, 3=80MHz)"
echo "enter (0/1/2/3, default 2):"
read input

if [ -z "$input" ]; then
    spi_speed=40
elif [ $input == 0 ]; then
    spi_speed=20
elif [ $input == 1 ]; then
    spi_speed=26.7
elif [ $input == 3 ]; then
    spi_speed=80
else
    spi_speed=40
fi

echo "spi speed: $spi_speed MHz"
echo ""

echo "STEP 4: choose spi mode(0=QIO, 1=QOUT, 2=DIO, 3=DOUT)"
echo "enter (0/1/2/3, default 0):"
read input

if [ -z "$input" ]; then
    spi_mode=QIO
elif [ $input == 1 ]; then
    spi_mode=QOUT
elif [ $input == 2 ]; then
    spi_mode=DIO
elif [ $input == 3 ]; then
    spi_mode=DOUT
else
    spi_mode=QIO
fi

echo "spi mode: $spi_mode"
echo ""

echo "STEP 5: choose spi size and map"
echo "    0= 512KB( 256KB+ 256KB)"
echo "    2=1024KB( 512KB+ 512KB)"
echo "    3=2048KB( 512KB+ 512KB)"
echo "    4=4096KB( 512KB+ 512KB)"
echo "    5=2048KB(1024KB+1024KB)"
echo "    6=4096KB(1024KB+1024KB)"
echo "    7=4096KB(2048KB+2048KB) not support ,just for compatible with nodeMCU board"
echo "    8=8192KB(1024KB+1024KB)"
echo "    9=16384KB(1024KB+1024KB)"
echo "enter (0/2/3/4/5/6/7/8/9, default 0):"
read input

if [ -z "$input" ]; then
    spi_size_map=0
    echo "spi size: 512KB"
    echo "spi ota map:  256KB + 256KB"
elif [ $input == 2 ]; then
    spi_size_map=2
    echo "spi size: 1024KB"
    echo "spi ota map:  512KB + 512KB"
elif [ $input == 3 ]; then
    spi_size_map=3
    echo "spi size: 2048KB"
    echo "spi ota map:  512KB + 512KB"
elif [ $input == 4 ]; then
    spi_size_map=4
    echo "spi size: 4096KB"
    echo "spi ota map:  512KB + 512KB"
elif [ $input == 5 ]; then
    spi_size_map=5
    echo "spi size: 2048KB"
    echo "spi ota map:  1024KB + 1024KB"
elif [ $input == 6 ]; then
    spi_size_map=6
    echo "spi size: 4096KB"
    echo "spi ota map:  1024KB + 1024KB"
elif [ $input == 7 ]; then
    spi_size_map=7
    echo"not support ,just for compatible with nodeMCU board"
    exit
elif [ $input == 8 ]; then
    spi_size_map=8
    echo "spi size: 8192KB"
    echo "spi ota map:  1024KB + 1024KB"
elif [ $input == 9 ]; then
    spi_size_map=9
    echo "spi size: 16384KB"
    echo "spi ota map:  1024KB + 1024KB"
else
    spi_size_map=0
    echo "spi size: 512KB"
    echo "spi ota map:  256KB + 256KB"
fi

touch user/user_main.c

echo ""
echo "start..."
echo ""

make COMPILE=gcc BOOT=$boot APP=$app SPI_SPEED=$spi_speed SPI_MODE=$spi_mode SPI_SIZE_MAP=$spi_size_map
