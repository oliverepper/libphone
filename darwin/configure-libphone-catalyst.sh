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

if [ ! -f "${INSTALL_DIR}/lib/libpjproject.a" ]; then
	print_error "The file \"${INSTALL_DIR}/lib/libpjproject.a\" does not exist. Please build pjproject, first."
	exit 1
fi

cmake -B"${BUILD_DIR}" \
	  -S"${SOURCE_DIR}" \
	  -G"${GENERATOR}" \
	  -DCMAKE_PREFIX_PATH="${INSTALL_DIR}" \
	  -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
	  -DCMAKE_TOOLCHAIN_FILE="$(dirname "$0")/../third-party/ios-cmake/ios.toolchain.cmake" \
	  -DPLATFORM=MAC_CATALYST_UNIVERSAL \
	  -DDEPLOYMENT_TARGET=13.1 \
	  -DCMAKE_MACOSX_BUNDLE=NO