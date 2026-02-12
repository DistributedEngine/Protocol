#!/bin/bash

# Copyright (c) 2025 — 2026 Ian Torres <iantorres@outlook.com>.
# All rights reserved.

set -e
set -o pipefail

# Configuration
BOOST_VERSION="1.90.0"
VARIANT=${1:-debug}
LINK=${2:-shared}
BUILD_THREADS=$(nproc)

if [ "$VARIANT" == "debug" ]; then
  CMAKE_BUILD_TYPE="Debug"
  BOOST_VARIANT="debug"
  BOOST_DEBUG_SYMBOLS="on"
else
  CMAKE_BUILD_TYPE="Release"
  BOOST_VARIANT="release"
  BOOST_DEBUG_SYMBOLS="off"
fi

if [ "$LINK" == "shared" ]; then
  BUILD_SHARED_LIBS="ON"
  BOOST_LINK="shared"
else
  BUILD_SHARED_LIBS="OFF"
  BOOST_LINK="static"
fi

echo "Setting up environment with VARIANT=$VARIANT, LINK=$LINK, THREADS=$BUILD_THREADS"

# 1. System Dependencies
echo "Installing system dependencies..."
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    bash \
    zip \
    unzip \
    tzdata \
    libtool \
    automake \
    m4 \
    re2c \
    supervisor \
    libssl-dev \
    zlib1g-dev \
    libcurl4-openssl-dev \
    protobuf-compiler \
    libprotobuf-dev \
    python3 \
    doxygen \
    graphviz \
    rsync \
    gcovr \
    lcov \
    autoconf \
    clang-tools \
    libunwind-dev \
    gnupg \
    binutils

# 2. libbcrypt
echo "Building libbcrypt ($CMAKE_BUILD_TYPE, $LINK)..."
git clone https://github.com/Zen0x7/libbcrypt.git bcrypt
cd bcrypt
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE -DBUILD_SHARED_LIBS=$BUILD_SHARED_LIBS
make -j$BUILD_THREADS
sudo make install
cd ../..
rm -rf bcrypt

# 3. spdlog
echo "Building spdlog (v1.16.0, $CMAKE_BUILD_TYPE, $LINK)..."
git clone https://github.com/gabime/spdlog.git spdlog
cd spdlog
git checkout tags/v1.16.0
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE -DSPDLOG_BUILD_SHARED=$BUILD_SHARED_LIBS
make -j$BUILD_THREADS
sudo make install
cd ../..
rm -rf spdlog

# 4. fmt
echo "Building fmt (12.1.0, $CMAKE_BUILD_TYPE, $LINK)..."
git clone https://github.com/fmtlib/fmt.git fmt
cd fmt
git checkout tags/12.1.0
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE -DBUILD_SHARED_LIBS=$BUILD_SHARED_LIBS -DFMT_DOC=OFF -DFMT_TEST=OFF -DFMT_FUZZ=OFF -DFMT_CUDA_TEST=OFF
make -j$BUILD_THREADS
sudo make install
cd ../..
rm -rf fmt

# 5. Boost
echo "Building Boost $BOOST_VERSION ($BOOST_VARIANT, $BOOST_LINK)..."
BOOST_VERSION_DASH=$(echo $BOOST_VERSION | sed 's/\./_/g')
wget https://archives.boost.io/release/$BOOST_VERSION/source/boost_$BOOST_VERSION_DASH.tar.gz
tar -xf boost_$BOOST_VERSION_DASH.tar.gz
cd boost_$BOOST_VERSION_DASH

./bootstrap.sh --with-libraries=all

sudo ./b2 install link=$BOOST_LINK runtime-link=$BOOST_LINK variant=$BOOST_VARIANT debug-symbols=$BOOST_DEBUG_SYMBOLS --without-python -j$BUILD_THREADS

cd ..
rm -rf boost_$BOOST_VERSION_DASH
rm boost_$BOOST_VERSION_DASH.tar.gz

echo "Environment setup complete!"
