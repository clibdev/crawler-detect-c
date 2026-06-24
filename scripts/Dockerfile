FROM ubuntu:26.04

WORKDIR /app

RUN apt-get update && apt-get install --no-install-recommends -y curl ca-certificates \
    && curl -sSo /etc/apt/trusted.gpg.d/kitware-key.asc https://apt.kitware.com/keys/kitware-archive-latest.asc \
    && echo "deb https://apt.kitware.com/ubuntu/ resolute main" > /etc/apt/sources.list.d/kitware.list \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
        git gcc g++ ninja-build nasm cmake \
    && rm -rf /var/lib/apt/lists/*

ENTRYPOINT ["/bin/bash"]
