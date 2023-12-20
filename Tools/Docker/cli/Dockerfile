FROM ubuntu:20.04

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

ARG SMING_REPO=https://github.com/SmingHub/Sming
ARG SMING_BRANCH=develop
ARG INSTALL_ARGS=all

RUN git clone $SMING_REPO -b $SMING_BRANCH /opt/sming

WORKDIR "/opt/sming"

RUN Tools/install.sh $INSTALL_ARGS
RUN echo ". $(pwd)/Tools/export.sh" >> ~/.bashrc
