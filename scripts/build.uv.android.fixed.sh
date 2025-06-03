#!/bin/bash

set -e  # Exit on error

UV_VERSION="1.49.2"
WORKING_DIR="/workspaces/AidRigCode"
DEPS_DIR="${WORKING_DIR}/deps_android"
BUILD_DIR="${WORKING_DIR}/build_uv_android"
UV_DIR="${BUILD_DIR}/libuv-v${UV_VERSION}"

echo "Creating directories..."
mkdir -p "${DEPS_DIR}/include"
mkdir -p "${DEPS_DIR}/lib"

echo "Cleaning old build..."
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo "Downloading and extracting libuv..."
wget "https://dist.libuv.org/dist/v${UV_VERSION}/libuv-v${UV_VERSION}.tar.gz"
tar xf "libuv-v${UV_VERSION}.tar.gz"

cd "${UV_DIR}"

echo "Setting up Android build..."
# Add include path for ifaddrs.h
echo "#include <ifaddrs.h>" > src/unix/android-ifaddrs.h

# Update tcp.c and linux.c to include the correct header
sed -i '1i\#include "android-ifaddrs.h"' src/unix/tcp.c
sed -i '1i\#include "android-ifaddrs.h"' src/unix/linux.c

echo "Configuring with CMake..."
# Generate build files with CMake for Android
cmake -Bbuild -H. \
    -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-24 \
    -DCMAKE_BUILD_TYPE=Release \
    -DLIBUV_BUILD_TESTS=OFF

echo "Building..."
# Build
cd build
make -j$(nproc) VERBOSE=1

echo "Copying artifacts..."
# Copy headers
cp -rv "${UV_DIR}/include/uv.h" "${DEPS_DIR}/include/" || echo "Failed to copy uv.h"
cp -rv "${UV_DIR}/include/uv" "${DEPS_DIR}/include/" || echo "Failed to copy uv directory"

# Copy libraries
cp -v libuv.a "${DEPS_DIR}/lib/" || echo "Failed to copy libuv.a"
cp -v libuv.so "${DEPS_DIR}/lib/" || echo "Failed to copy libuv.so"

echo "Verifying installation..."
ls -lR "${DEPS_DIR}"