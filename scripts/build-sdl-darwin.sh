#!/bin/bash
# Oliver Epper <oliver.epper@gmail.com>

set -e

if [ $# -eq 0 ]
then
    echo "sh $0 <absolute path>"
    exit 1
fi

PREFIX=$1
BUILD_BASE_DIR=build-sdl
SDL_VERSION=release-2.26.4

if [ -d SDL ]
then
    pushd SDL
    git clean -fxd
    git reset --hard $SDL_VERSION
    popd
else
    git -c advice.detachedHead=false clone --depth 1 --branch $SDL_VERSION https://github.com/libsdl-org/SDL.git
fi

function build {
    local TOOLCHAIN_PLATFORM_NAME=$1
    local INSTALL_PREFIX=$2
    local DEPLOYMENT_TARGET=$3
    local BUILD_DIR="${BUILD_BASE_DIR}"/"${TOOLCHAIN_PLATFORM_NAME}"

    echo "Building for platform ${TOOLCHAIN_PLATFORM_NAME} with deployment target ${DEPLOYMENT_TARGET}"
    echo "Building in ${BUILD_DIR}"
    echo "Installing to: ${INSTALL_PREFIX}"

    case "${TOOLCHAIN_PLATFORM_NAME}" in
        *"CATALYST"*|"MAC")
            cmake -B"${BUILD_DIR}" \
                -SSDL \
                -G"Ninja Multi-Config" \
                -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
                -DCMAKE_TOOLCHAIN_FILE=../ios-cmake/ios.toolchain.cmake \
                -DPLATFORM="${TOOLCHAIN_PLATFORM_NAME}" \
                -DENABLE_BITCODE=OFF \
                -DSDL_HAPTIC=OFF \
                -DSDL_HIDAPI_JOYSTICK=OFF \
                -DSDL_SYSTEM_ICONV=OFF \
                -DSDL_JOYSTICK=OFF
            ;;
        *)
            cmake -B"${BUILD_DIR}" \
                -SSDL \
                -G"Ninja Multi-Config" \
                -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
                -DCMAKE_TOOLCHAIN_FILE=../ios-cmake/ios.toolchain.cmake \
                -DPLATFORM="${TOOLCHAIN_PLATFORM_NAME}" \
                -DCMAKE_OSX_DEPLOYMENT_TARGET="${DEPLOYMENT_TARGET}" \
                -DENABLE_BITCODE=OFF \
                -DSDL_HAPTIC=OFF \
                -DSDL_HIDAPI_JOYSTICK=OFF \
                -DSDL_SYSTEM_ICONV=OFF \
                -DSDL_JOYSTICK=OFF
            ;;
    esac

    cmake --build "${BUILD_DIR}" \
        --config Release \
        --target install
}

# build for macOS on arm64
MACOS_ARM64_INSTALL_PREFIX="${PREFIX}/macos-arm64"
build MAC_ARM64 "${MACOS_ARM64_INSTALL_PREFIX}" 11.0

# build for macOS on x86_64
MACOS_X86_64_INSTALL_PREFIX="${PREFIX}/macos-x86_64"
build MAC "${MACOS_X86_64_INSTALL_PREFIX}" 11.0

# build macOS universal
MACOS_UNIVERSAL_INSTALL_PREFIX="${PREFIX}/macos-universal"
build MAC_UNIVERSAL "${MACOS_UNIVERSAL_INSTALL_PREFIX}" 11.0
