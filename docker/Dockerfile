# ------------------------------------
# Fast dockerized development environment 
# for the Sming Framework: https://github.com/SmingHub/Sming.git
# ------------------------------------
FROM attachix/c9-esp8266-sdk:latest
MAINTAINER Slavey Karadzhov <slav@attachix.com>

RUN cd /workspace && git clone https://github.com/SmingHub/Sming.git 

ENV SMING_HOME /workspace/Sming/Sming

ENTRYPOINT /usr/bin/supervisord
