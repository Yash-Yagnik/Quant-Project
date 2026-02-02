# Lumina-HFT: Multi-Threaded Liquidity Provision Framework
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libfmt-dev \
    python3-dev \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN mkdir build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_PYBIND11=OFF -DUSE_AVX512=OFF \
    && cmake --build . -j$(nproc) \
    && ctest --output-on-failure

WORKDIR /app/build
CMD ["./lumina_hft_main"]
