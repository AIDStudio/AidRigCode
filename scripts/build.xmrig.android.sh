#!/bin/bash

set -e  # Exit on error

WORKING_DIR="/workspaces/AidRigCode"
DEPS_DIR="${WORKING_DIR}/deps_android"
BUILD_DIR="${WORKING_DIR}/build_android"

# Create build directory
echo "Creating build directory..."
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Configure with CMake
echo "Configuring with CMake..."
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-24 \
    -DCMAKE_BUILD_TYPE=Release \
    -DXMRIG_DEPS=${DEPS_DIR} \
    -DWITH_HWLOC=OFF \
    -DWITH_TLS=OFF \
    -DWITH_OPENCL=OFF \
    -DWITH_CUDA=OFF \
    -DWITH_MSR=OFF \
    -DWITH_ADL=OFF \
    -DWITH_NVML=OFF \
    -DWITH_ASM=OFF \
    -DWITH_SSE4_1=OFF \
    -DWITH_AVX2=OFF \
    -DWITH_VAES=OFF \
    -DWITH_BENCHMARK=ON \
    -DWITH_DMI=OFF \
    -DARM_V8=ON \
    -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld"

# Build
echo "Building..."
make -j$(nproc) VERBOSE=1

echo "Build complete. Binary location: ${BUILD_DIR}/aidrig"
