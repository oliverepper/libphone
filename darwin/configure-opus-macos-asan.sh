#!/usr/bin/env bash
# Oliver Epper <oliver.epper@gmail.com>

source "$(dirname "$0")"/functions.sh

if [ $# -ne 4 ]; then
	print_error "Usage: $0 <generator> <build-dir> <source-dir> <install-dir>"
	exit 1
fi

GENERATOR=$1
BUILD_DIR=$2
SOURCE_DIR=$3
INSTALL_DIR=$4

cmake -B"${BUILD_DIR}" \
	  -S"${SOURCE_DIR}" \
	  -G"${GENERATOR}" \
	  -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
	  -DCMAKE_TOOLCHAIN_FILE="$(dirname "$0")/../third-party/ios-cmake/ios.toolchain.cmake" \
	  -DPLATFORM=MAC_UNIVERSAL \
	  -DDEPLOYMENT_TARGET=11 \
	  -DCMAKE_MACOSX_BUNDLE=NO \
	  -DOPUS_INSTALL_CMAKE_CONFIG_MODULE=OFF \
	  -DOPUS_FORTIFY_SOURCE=0 \
	  -DCMAKE_C_FLAGS="-fsanitize=address" \
	  -DCMAKE_CXX_FLAGS="-fsanitize=address" \
	  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"