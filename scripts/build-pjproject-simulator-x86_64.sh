#!/bin/bash
# Oliver Epper <oliver.epper@gmail.com>

source "$(dirname "$0")"/build-pjproject-darwin-base.sh

PREFIX="${PREFIX}/simulator-x86_64"
rm -rf "${PREFIX}"

pushd pjproject || exit 1
prepare YES

OPUS=("$(pwd)"/../../installed/opus/simulator-x86_64)
OPUS_LATEST=${OPUS[${#OPUS[@]} - 1]}
if [[ -d "${OPUS_LATEST}" ]]
then
    CONFIGURE_EXTRA_PARAMS+=("--with-opus=${OPUS_LATEST}")
fi

SDKPATH=$(xcrun -sdk iphonesimulator --show-sdk-path)
ARCH="x86_64"
CFLAGS="-isysroot $SDKPATH -miphonesimulator-version-min=13 -DPJ_SDK_NAME=\"\\\"$(basename "$SDKPATH")\\\"\" -arch $ARCH" \
LDFLAGS="-isysroot $SDKPATH -framework AudioToolbox -framework Foundation -framework Network -framework Security -arch $ARCH" \
./aconfigure --prefix="${PREFIX}" --host="${ARCH}"-apple-darwin_ios "${CONFIGURE_EXTRA_PARAMS[@]}" --disable-sdl

make dep && make clean
make
make install

create_lib "${PREFIX}/lib"
pkgconfig_ios "${PREFIX}"
popd || exit 1
