FROM phusion/baseimage:0.9.18

# ------------------------------------------------------------------------------
# Install Dependencies
# ------------------------------------------------------------------------------

RUN apt-get update && \
    apt-get install -y make \
	wget \
	unrar \ 
	autoconf \ 
	automake  \
	libtool \
	gcc \
	g++ \
	gperf \
	flex \
	bison \
	texinfo \
	gawk \
	ncurses-dev \ 
	libexpat-dev \ 
	python \
	python-dev \
	python-serial \ 
	sed \
	git \
	unzip \ 
	bash \	
	wget \
	bzip2 \
	vim \
	screen \ 
	sudo \
	help2man \
	curl

# ------------------------------------------------------------------------------	
# Install Node
# ------------------------------------------------------------------------------


RUN set -ex \
  && for key in \
    9554F04D7259F04124DE6B476D5A82AC7E37093B \
    94AE36675C464D64BAFA68DD7434390BDBE9B9C5 \
    0034A06D9D9B0064CE8ADF6BF1747F4AD2306D93 \
    FD3A5288F042B6850C66B31F09FE44734EB7990E \
    71DCFD284A79C3B38668286BC97EC7A07EDE3FC1 \
    DD8F2338BAE7501E3DD5AC78C273792F7D83545D \
    B9AE9905FFD7803F25714661B63B535A4C206CA9 \
    C4F0DFFF4E8C1A8236409D08E73BC641CC11F4C8 \
  ; do \
    gpg --keyserver ha.pool.sks-keyservers.net --recv-keys "$key"; \
  done

ENV NPM_CONFIG_LOGLEVEL info
ENV NODE_VERSION 4.4.3

RUN curl -SLO "https://nodejs.org/dist/v$NODE_VERSION/node-v$NODE_VERSION-linux-x64.tar.xz"
RUN curl -SLO "https://nodejs.org/dist/v$NODE_VERSION/SHASUMS256.txt.asc"
RUN gpg --batch --decrypt --output SHASUMS256.txt SHASUMS256.txt.asc
RUN grep " node-v$NODE_VERSION-linux-x64.tar.xz\$" SHASUMS256.txt | sha256sum -c -
RUN tar -xJf "node-v$NODE_VERSION-linux-x64.tar.xz" -C /usr/local --strip-components=1
RUN rm "node-v$NODE_VERSION-linux-x64.tar.xz" SHASUMS256.txt.asc SHASUMS256.txt


# ------------------------------------------------------------------------------
# Install Cloud9
# ------------------------------------------------------------------------------

RUN git clone https://github.com/c9/core.git /opt/cloud9 && cd /opt/cloud9 && git checkout 32fc12bf922288d0c2b518c73aa75e19f14bed8b
WORKDIR /opt/cloud9
RUN scripts/install-sdk.sh
RUN npm install

# ------------------------------------------------------------------------------
# Tweak standlone.js conf
# ------------------------------------------------------------------------------

RUN sed -i -e 's_127.0.0.1_0.0.0.0_g' /opt/cloud9/configs/standalone.js 

# ------------------------------------------------------------------------------
# Install Supervisor.
# ------------------------------------------------------------------------------

RUN apt-get install -y supervisor
RUN sed -i 's/^\(\[supervisord\]\)$/\1\nnodaemon=true/' /etc/supervisor/supervisord.conf

# ------------------------------------------------------------------------------
# Add supervisord configs
# ------------------------------------------------------------------------------

ADD conf/* /etc/supervisor/conf.d/

# ------------------------------------------------------------------------------
# Clean up APT when done.
# ------------------------------------------------------------------------------

RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# ------------------------------------------------------------------------------
# Install ESP8266 SDK
# ------------------------------------------------------------------------------

RUN useradd -d /home/espbuilder -m espbuilder && \
	usermod -a -G dialout espbuilder && \
	mkdir -p /etc/sudoers.d && \
	echo "espbuilder ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/espbuilder && \
	chmod 0440 /etc/sudoers.d/espbuilder

RUN cd /opt && git clone --recursive https://github.com/pfalcon/esp-open-sdk.git && cd /opt/esp-open-sdk && git checkout e8d757b1a70a5cf19df0afe23a769739c6cff343
RUN chmod 777 -R /opt/esp-open-sdk

USER espbuilder
RUN cd /opt/esp-open-sdk && make VENDOR_SDK=1.5.4 STANDALONE=y

USER root

# ------------------------------------------------------------------------------
# Cleanup
# ------------------------------------------------------------------------------

RUN rm -rf /home/espbuilder/esp-open-sdk/crosstool-NG/.build

# ------------------------------------------------------------------------------
# Set Environment
# ------------------------------------------------------------------------------

ENV PATH /opt/esp-open-sdk/xtensa-lx106-elf/bin:$PATH
ENV XTENSA_TOOLS_ROOT /opt/esp-open-sdk/xtensa-lx106-elf/bin
ENV SDK_BASE /opt/esp-open-sdk/sdk
ENV FW_TOOL /opt/esp-open-sdk/xtensa-lx106-elf/bin/esptool.py  

ENV ESP_HOME /opt/esp-open-sdk
ENV SMING_HOME /opt/sming/Sming

# ------------------------------------------------------------------------------
# Install esptool.py
# ------------------------------------------------------------------------------

RUN wget https://github.com/themadinventor/esptool/archive/master.zip && unzip master.zip && mv esptool-master $ESP_HOME/esptool && rm master.zip

# ------------------------------------------------------------------------------
# Install esptool2
# ------------------------------------------------------------------------------

RUN cd $ESP_HOME && git clone https://github.com/raburton/esptool2 && cd $ESP_HOME/esptool2 && git checkout ec0e2c72952f4fa8242eedd307c58a479d845abe
RUN cd $ESP_HOME/esptool2 && make

ENV PATH $ESP_HOME/esptool2:$PATH


EXPOSE 80

CMD ["supervisord", "-c", "/etc/supervisor/supervisord.conf"]
