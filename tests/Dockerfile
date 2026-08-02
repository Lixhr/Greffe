# Usage:
#   docker build -t greffe-tests tests/
#   docker run --rm -v "$(pwd)/tests":/workspace greffe-tests        # `make re`, writes into tests/*/build on host
#   docker run --rm -v "$(pwd)/tests":/workspace greffe-tests clean  # overrides CMD

FROM debian@sha256:63a496b5d3b99214b39f5ed70eb71a61e590a77979c79cbee4faf991f8c0783e

ARG AARCH64_LE_URL=https://developer.arm.com/-/media/Files/downloads/gnu/15.2.rel1/binrel/arm-gnu-toolchain-15.2.rel1-x86_64-aarch64-none-linux-gnu.tar.xz
ARG AARCH64_BE_URL=https://developer.arm.com/-/media/Files/downloads/gnu/15.2.rel1/binrel/arm-gnu-toolchain-15.2.rel1-x86_64-aarch64_be-none-linux-gnu.tar.xz
ARG ARM_BE_URL=https://toolchains.bootlin.com/downloads/releases/toolchains/armebv7-eabihf/tarballs/armebv7-eabihf--glibc--stable-2024.02-1.tar.bz2
ARG ARM_LE_URL=https://toolchains.bootlin.com/downloads/releases/toolchains/armv7-eabihf/tarballs/armv7-eabihf--glibc--stable-2025.08-1.tar.xz

ENV TOOLCHAIN_DIR=/opt/toolchains
WORKDIR $TOOLCHAIN_DIR

RUN apt-get update && \
    apt-get install -y --no-install-recommends curl make xz-utils bzip2 ca-certificates && \
    rm -rf /var/lib/apt/lists/*

RUN curl -fSL -o aarch64-le.tar.xz "$AARCH64_LE_URL" && \
    tar -xJf aarch64-le.tar.xz && rm aarch64-le.tar.xz && \
    mv arm-gnu-toolchain-15.2.rel1-x86_64-aarch64-none-linux-gnu aarch64-le_linux && \
    \
    curl -fSL -o aarch64-be.tar.xz "$AARCH64_BE_URL" && \
    tar -xJf aarch64-be.tar.xz && rm aarch64-be.tar.xz && \
    mv arm-gnu-toolchain-15.2.rel1-x86_64-aarch64_be-none-linux-gnu aarch64-be_linux && \
    \
    curl -fSL -o arm-be.tar.bz2 "$ARM_BE_URL" && \
    tar -xjf arm-be.tar.bz2 && rm arm-be.tar.bz2 && \
    mv armebv7-eabihf--glibc--stable-2024.02-1 arm-be_linux && \
    \
    curl -fSL -o arm-le.tar.bz2 "$ARM_LE_URL" && \
    tar -xJf arm-le.tar.bz2 && rm arm-le.tar.bz2 && \
    mv armv7-eabihf--glibc--stable-2025.08-1 arm-le_linux 

WORKDIR /workspace

ENTRYPOINT ["make"]
CMD ["re"]