#!/bin/bash
# Oliver Epper <oliver.epper@gmail.com>

source "$(dirname "$0")"/build-pjproject-darwin-base.sh

PREFIX="${PREFIX}/macos-arm64"
rm -rf "${PREFIX}"

pushd pjproject || exit 1
prepare NO YES

OPUS=("$(pwd)"/../../installed/opus/macos-arm64)
OPUS_LATEST=${OPUS[${#OPUS[@]} - 1]}
if [[ -d "${OPUS_LATEST}" ]]
then
    CONFIGURE_EXTRA_PARAMS+=("--with-opus=${OPUS_LATEST}")
fi

SDL=("$(pwd)"/../../installed/sdl/macos-arm64)
SDL_LATEST=${SDL[${#SDL[@]} - 1]}
if [[ -d "${SDL_LATEST}" ]]; then
    CONFIGURE_EXTRA_PARAMS+=("--with-sdl=${SDL_LATEST}")
fi

SDKPATH=$(xcrun -sdk macosx --show-sdk-path)
ARCH="arm"
CFLAGS="-isysroot $SDKPATH -mmacosx-version-min=11 -DPJ_SDK_NAME=\"\\\"$(basename "$SDKPATH")\\\"\"" \
LDFLAGS="-isysroot $SDKPATH -framework AudioToolbox -framework Foundation -framework Network -framework Security" \
./aconfigure --prefix="${PREFIX}" --host="${ARCH}"-apple-darwin "${CONFIGURE_EXTRA_PARAMS[@]}"

make dep && make clean
make
make install

create_lib "${PREFIX}/lib"
pkgconfig_macos "${PREFIX}"
popd || exit 1

