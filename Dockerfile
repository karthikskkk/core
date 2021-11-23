FROM phusion/baseimage:0.11
MAINTAINER The dxperts decentralized organisation

ENV LANG=en_US.UTF-8
RUN \
    apt-get update -y && \
    apt-get install -y \
      g++ \
      autoconf \
      cmake \
      git \
      libbz2-dev \
      libcurl4-openssl-dev \
      libssl-dev \
      libncurses-dev \
      libboost-thread-dev \
      libboost-iostreams-dev \
      libboost-date-time-dev \
      libboost-system-dev \
      libboost-filesystem-dev \
      libboost-program-options-dev \
      libboost-chrono-dev \
      libboost-test-dev \
      libboost-context-dev \
      libboost-regex-dev \
      libboost-coroutine-dev \
      libtool \
      doxygen \
      ca-certificates \
      fish \
    && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

ADD . /dxperts-core
WORKDIR /dxperts-core

# Compile
RUN \
    ( git submodule sync --recursive || \
      find `pwd`  -type f -name .git | \
	while read f; do \
	  rel="$(echo "${f#$PWD/}" | sed 's=[^/]*/=../=g')"; \
	  sed -i "s=: .*/.git/=: $rel/=" "$f"; \
	done && \
      git submodule sync --recursive ) && \
    git submodule update --init --recursive && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
	-DGRAPHENE_DISABLE_UNITY_BUILD=ON \
        . && \
    make blockproducer_node cli_wallet get_dev_key && \
    install -s programs/blockproducer_node/blockproducer_node programs/genesis_util/get_dev_key programs/cli_wallet/cli_wallet /usr/local/bin && \
    #
    # Obtain version
    mkdir /etc/dxperts && \
    git rev-parse --short HEAD > /etc/dxperts/version && \
    cd / && \
    rm -rf /dxperts-core

# Home directory $HOME
WORKDIR /
RUN useradd -s /bin/bash -m -d /var/lib/dxperts dxperts
ENV HOME /var/lib/dxperts
RUN chown dxperts:dxperts -R /var/lib/dxperts

# Volume
VOLUME ["/var/lib/dxperts", "/etc/dxperts"]

# rpc service:
EXPOSE 8090
# p2p service:
EXPOSE 1776

# default exec/config files
ADD docker/default_config.ini /etc/dxperts/config.ini
ADD docker/default_logging.ini /etc/dxperts/logging.ini
ADD docker/dxpertsentry.sh /usr/local/bin/dxpertsentry.sh
RUN chmod a+x /usr/local/bin/dxpertsentry.sh

# Make Docker send SIGINT instead of SIGTERM to the daemon
STOPSIGNAL SIGINT

# default execute entry
CMD ["/usr/local/bin/dxpertsentry.sh"]
