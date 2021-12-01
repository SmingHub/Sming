#!/bin/bash
#
# This file should be sourced after pulling in Sming repo, i.e:
#
#    . /opt/sming/Tools/install.sh
#

[ "$0" = "$BASH_SOURCE" ]; sourced=$?

inst_host=0
inst_doc=0
inst_esp8266=0
inst_esp32=0
inst_rp2040=0
err=0

FONT_PACKAGES="fonts-ubuntu fonts-noto-mono xfonts-base fonts-urw-base35 fonts-droid-fallback"

for opt in "$@"; do
    case $opt in
        all)
            inst_host=1
            inst_esp8266=1
            inst_esp32=1
            inst_rp2040=1
            ;;

        host | doc | esp8266 | esp32 | rp2040)
            eval "inst_$opt=1"
            ;;

        fonts)
            EXTRA_PACKAGES+=" $FONT_PACKAGES"
            ;;

        *)
            echo "Unknown option '$opt'"
            err=1
            ;;
    esac
done

if [[ $err -eq 1 ]] || [ $# -eq 0 ]; then
    echo 'Sming Installation options:'
    echo '   host      Host development tools'
    echo '   esp8266   ESP8266 development tools'
    echo '   esp32     ESP32 development tools'
    echo '   rp2040    RP2040 tools (Raspberry Pi Pico)'
    echo '   all       Install all architectures'
    echo '   doc       Tools required to build documentation'
    echo '   fonts     Install fonts used by Graphics library (normally included with Ubuntu)'
    echo
    if [ $sourced = 1 ]; then
        return 1
    else
        exit 1
    fi
fi

# Sming repository for binary archives
SMINGTOOLS=https://github.com/SmingHub/SmingTools/releases/download/1.0

# Set default environment variables and WGET options
if [ -z "$APPVEYOR" ]; then
    source $(dirname "$BASH_SOURCE")/export.sh

    # Ensure default path is writeable
    sudo mkdir -p /opt
    sudo chown $USER:$USER /opt

    WGET="wget"
else
    # Don't clutter up logfiles for CI builds
    WGET="wget --no-verbose"
fi

# Installers put downloaded archives here
DOWNLOADS="downloads"
mkdir -p $DOWNLOADS

# Identify package installer for distribution
if [ -n "$(grep debian /etc/os-release)" ]; then
    DIST=debian
    PKG_INSTALL="sudo apt-get install -y"
elif [ -n "$(grep fedora /etc/os-release)" ]; then
    DIST=fedora
    PKG_INSTALL="sudo dnf install -y"
else
    echo "Unsupported distribution"
    if [ $sourced = 1 ]; then
        return 1
    else
        exit 1
    fi
fi

# Common install

if [ -n "$APPVEYOR" ] || [ -n "$GITHUB_ACTION" ]; then

    sudo apt-get -y update
    $PKG_INSTALL \
        clang-format-8 \
        g++-9-multilib \
        python3-setuptools \
        $EXTRA_PACKAGES

    sudo update-alternatives --set gcc /usr/bin/gcc-9

else

    case $DIST in
        debian)
            sudo apt-get -y update || echo "Update failed... Try to install anyway..."
            $PKG_INSTALL \
                cmake \
            	curl \
            	git \
            	make \
                unzip \
                g++ \
            	g++-multilib \
            	python3 \
            	python3-pip \
            	python3-setuptools \
                wget \
                $EXTRA_PACKAGES

            $PKG_INSTALL clang-format-8 || printf "\nWARNING: Failed to install optional clang-format-8.\n\n"
            ;;

        fedora)
            $PKG_INSTALL \
                cmake \
                gawk \
                gcc \
                gcc-c++ \
                gettext \
                git \
                glibc-devel.i686 \
                libstdc++.i686 \
                make \
                python3 \
                python3-pip \
                sed \
                unzip \
                wget
            ;;

    esac

    sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 100

fi

set -e

if [ -f "/usr/bin/clang-format-8" ]; then
    sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-8 100
fi

python3 -m pip install --upgrade pip protobuf -r "$SMING_HOME/../Tools/requirements.txt"


install() {
    echo
    echo
    echo "** Installing $1 toolchain"
    echo
    source "$SMING_HOME/Arch/$1/Tools/install.sh"
}

if [ $inst_host -eq 1 ]; then
    install Host
fi

if [ $inst_doc -eq 1 ]; then
    source "$SMING_HOME/../docs/Tools/install.sh"
fi

if [ $inst_esp8266 -eq 1 ]; then
    install Esp8266
fi

if [ $inst_esp32 -eq 1 ]; then
    install Esp32
fi

if [ $inst_rp2040 -eq 1 ]; then
    install Rp2040
fi

if [ -z "$KEEP_DOWNLOADS" ]; then
    rm -f "$DOWNLOADS/*"
fi


echo
echo Installation complete
echo

if [ $sourced != 1 ]; then
    echo "You may need to set environment variables:"
    echo "  source $SMING_HOME/../Tools/export.sh"
    echo
fi
