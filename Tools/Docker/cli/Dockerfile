FROM ubuntu

# ------------------------------------------------------------------------------
# Set Environment
# ------------------------------------------------------------------------------

# Common
ENV SMING_HOME /opt/Sming/Sming
ENV PYTHON /usr/bin/python3

# Esp8266
#ENV UDK_ROOT /opt/esp-open-sdk
ENV EQT_ROOT /opt/esp-quick-toolchain
ENV ESP_HOME $EQT_ROOT

# Esp32
ENV IDF_PATH /opt/esp-idf
ENV IDF_TOOLS_PATH /opt/esp32
ENV ESP32_PYTHON_PATH $PYTHON

# ------------------------------------------------------------------------------
# Pre-requisites
# ------------------------------------------------------------------------------

RUN apt-get -y update \
    && DEBIAN_FRONTEND=noninteractive \
    TZ=Europe/London \
    apt-get install -y \
    git \
    sudo \
    tzdata

# ------------------------------------------------------------------------------
# Fetch Sming and install tools
# ------------------------------------------------------------------------------

ARG SMING_BRANCH=develop
ARG SMING_REPO=SmingHub/Sming
ARG INSTALL_ARGS=all

RUN git clone -b $SMING_BRANCH -- https://github.com/$SMING_REPO $(readlink -m $SMING_HOME/..)

RUN $SMING_HOME/../Tools/install.sh $INSTALL_ARGS
