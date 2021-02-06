#!/bin/bash -e
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
err=0

for opt in "$@"; do
    case $opt in
        all)
            inst_host=1
            inst_doc=1
            inst_esp8266=1
            inst_esp32=1
            ;;

        host | doc | esp8266 | esp32)
            eval "inst_$opt=1"
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
    echo '   doc       Tools required to build documentation'
    echo '   esp8266   ESP8266 development tools'
    echo '   esp32     ESP32 development tools'
    echo '   all       Install everything'
    echo
    if [ $sourced = 1 ]; then
        return 1
    else
        exit 1
    fi
fi

if [ $sourced != 1 ]; then
    echo "Please source this script:"
    echo "  source $0"
    exit 1
fi

SMINGTOOLS=https://github.com/SmingHub/SmingTools/releases/download/1.0

if [ -z "$APPVEYOR" ]; then
    source $(dirname $BASH_SOURCE)/export.sh
fi

# Common install

sudo apt-get -y update

if [ -n "$APPVEYOR" ]; then

    sudo apt-get install -y \
        clang-format-6.0 \
        g++-9-multilib \
        python3-setuptools

    sudo update-alternatives --set gcc /usr/bin/gcc-9

else

    sudo apt-get install -y \
        clang-format-6.0 \
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
        wget

    sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 100

fi

sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-6.0 100

python3 -m pip install --upgrade pip -r $SMING_HOME/../Tools/requirements.txt

install() {
    source $SMING_HOME/Arch/$1/Tools/install.sh
}

if [ $inst_host -eq 1 ]; then
    install Host
fi

if [ $inst_doc -eq 1 ]; then
    source $SMING_HOME/../docs/Tools/install.sh
fi

if [ $inst_esp8266 -eq 1 ]; then
    install Esp8266
fi

if [ $inst_esp32 -eq 1 ]; then
echo    install Esp32
fi
