# ------------------------------------
# Fast dockerized development environment 
# for the Sming Framework: https://github.com/SmingHub/Sming.git
# ------------------------------------
FROM attachix/c9-esp8266-sdk:latest
MAINTAINER Slavey Karadzhov <slav@attachix.com>

COPY assets/welcome.html /cloud9/plugins/c9.ide.welcome/welcome.html
COPY assets/welcome.js /cloud9/plugins/c9.ide.welcome/welcome.js

RUN git clone https://github.com/SmingHub/Sming.git /workspace/Sming 

ENV SMING_HOME /workspace/Sming/Sming

ENTRYPOINT /usr/bin/supervisord
