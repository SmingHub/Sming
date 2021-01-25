# ------------------------------------
# Fast dockerized development environment 
# for the Sming Framework: https://github.com/SmingHub/Sming.git
# ------------------------------------
FROM docker_sming-cli
MAINTAINER Slavey Karadzhov "slav@attachix.com"

# ------------------------------------------------------------------------------
# Install Cloud9 and Supervisor
# ------------------------------------------------------------------------------

RUN apt-get update -y \
    && apt-get install -y \
        apache2-utils \
      	libxml2-dev \
        locales-all \
      	npm \
      	nodejs \
      	sshfs \
      	supervisor \
      	tmux

RUN git clone https://github.com/c9/core.git /cloud9 \
    && curl -s -L https://raw.githubusercontent.com/c9/install/master/install.sh | bash \
    && /cloud9/scripts/install-sdk.sh \
    && sed -i -e 's_127.0.0.1_0.0.0.0_g' /cloud9/configs/standalone.js \
    && mkdir -p /var/log/supervisor

ADD supervisord.conf /etc/

# VOLUME /workspace

EXPOSE 80

SHELL ["/bin/bash", "-c"]

CMD ["/usr/bin/supervisord", "--nodaemon", "--configuration", "/etc/supervisord.conf"]

COPY assets/welcome.html /cloud9/plugins/c9.ide.welcome/welcome.html
COPY assets/welcome.js /cloud9/plugins/c9.ide.welcome/welcome.js

ENTRYPOINT /usr/bin/supervisord
