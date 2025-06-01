#!/bin/bash
NDK_VERSION=r26d
NDK_ZIP=android-ndk-${NDK_VERSION}-linux.zip
NDK_DIR=android-ndk-${NDK_VERSION}

if [ ! -d "$NDK_DIR" ]; then
  echo "Downloading Android NDK $NDK_VERSION..."
  wget https://dl.google.com/android/repository/$NDK_ZIP
  unzip -q $NDK_ZIP
fi
