#!/usr/bin/env bash
# Oliver Epper <oliver.epper@gmail.com>

set -e
env -i

if [ $# -eq 0 ]
then
    echo "sh $0 <absolute path>"
    exit 1
fi

PREFIX=$1
BUILD_BASE_DIR=build-opus
OPUS_COMMIT=8cf872a186b96085b1bb3a547afd598354ebeb87

if [ -d opus ]
then
    pushd opus
    git clean -fxd
    git reset --hard "${OPUS_COMMIT}"
    popd
else
    # opus cmake script use .git, so check it out completely
    git clone https://gitlab.xiph.org/xiph/opus.git
    git -c advice.detachedHead=false -C opus checkout ${OPUS_COMMIT}
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
                -Sopus \
                -G"Ninja Multi-Config" \
                -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
                -DCMAKE_TOOLCHAIN_FILE=../ios-cmake/ios.toolchain.cmake \
                -DPLATFORM="${TOOLCHAIN_PLATFORM_NAME}" \
                -DENABLE_BITCODE=OFF \
                -DOPUS_INSTALL_CMAKE_CONFIG_MODULE=OFF
            ;;
        *)
            cmake -B"${BUILD_DIR}" \
                -Sopus \
                -G"Ninja Multi-Config" \
                -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
                -DCMAKE_TOOLCHAIN_FILE=../ios-cmake/ios.toolchain.cmake \
                -DPLATFORM="${TOOLCHAIN_PLATFORM_NAME}" \
                -DCMAKE_OSX_DEPLOYMENT_TARGET="${DEPLOYMENT_TARGET}" \
                -DENABLE_BITCODE=OFF \
                -DOPUS_INSTALL_CMAKE_CONFIG_MODULE=OFF
            ;;
    esac

    cmake --build "${BUILD_DIR}" \
        --config Release \
        --target install
}


# build for iOS on arm64
IOS_ARM64_INSTALL_PREFIX="${PREFIX}/ios-arm64"
build OS64 "${IOS_ARM64_INSTALL_PREFIX}" 13.0

# build for iOS simulator on arm64
SIMULATOR_ARM64_INSTALL_PREFIX="${PREFIX}/simulator-arm64"
build SIMULATORARM64 "${SIMULATOR_ARM64_INSTALL_PREFIX}" 13.0

# build for iOS simulator on x86_64
SIMULATOR_X86_64_INSTALL_PREFIX="${PREFIX}/simulator-x86_64"
build SIMULATOR64 "${SIMULATOR_X86_64_INSTALL_PREFIX}" 13.0

# build for Catalyst on arm64
MACCATALYST_ARM64_INSTALL_PREFIX="${PREFIX}/maccatalyst-arm64"
build MAC_CATALYST_ARM64 "${MACCATALYST_ARM64_INSTALL_PREFIX}" 13.1

# build for Catalyst on x86_64
MACCATALYST_X86_64_INSTALL_PREFIX="${PREFIX}/maccatalyst-x64_64"
build MAC_CATALYST "${MACCATALYST_X86_64_INSTALL_PREFIX}" 13.1

# build for macOS on arm64
MACOS_UNIVERSAL_INSTALL_PREFIX="${PREFIX}/macos-arm64"
build MAC_ARM64 "${MACOS_UNIVERSAL_INSTALL_PREFIX}" 11.0

# build for macOS on x86_64
MACOS_X86_64_INSTALL_PREFIX="${PREFIX}/macos-x86_64"
build MAC "${MACOS_X86_64_INSTALL_PREFIX}" 11.0
