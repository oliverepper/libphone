#!/usr/bin/env bash
# Oliver Epper <oliver.epper@gmail.com>

set -e

IOS_TOOLCHAIN_COMMIT=04d91f6675dabb3c97df346a32f6184b0a7ef845

if [ -d ios-cmake ]
then
    pushd ios-cmake
    git clean -fxd
    git reset --hard "${IOS_TOOLCHAIN_IOS_TOOLCHAIN_COMMIT}"
    popd
else
    git clone https://github.com/leetal/ios-cmake.git
    git -c advice.detachedHead=false -C ios-cmake checkout ${IOS_TOOLCHAIN_COMMIT}
fi
