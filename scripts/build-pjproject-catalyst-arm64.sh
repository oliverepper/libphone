#!/bin/bash
# Oliver Epper <oliver.epper@gmail.com>

source "$(dirname "$0")"/build-pjproject-darwin-base.sh

PREFIX="${PREFIX}/catalyst-arm64"
rm -rf "${PREFIX}"

pushd pjproject || exit 1
prepare YES

OPUS=("$(pwd)"/../../installed/opus/maccatalyst-arm64)
OPUS_LATEST=${OPUS[${#OPUS[@]} - 1]}
if [[ -d "${OPUS_LATEST}" ]]
then
    CONFIGURE_EXTRA_PARAMS+=("--with-opus=${OPUS_LATEST}")
fi

SDKPATH=$(xcrun -sdk macosx --show-sdk-path)
ARCH="arm64"
CFLAGS="-isysroot $SDKPATH -isystem ${SDKPATH}/System/iOSSupport/usr/include -iframework ${SDKPATH}/System/iOSSupport/System/Library/Frameworks -miphoneos-version-min=13.1 -DPJ_SDK_NAME=\"\\\"$(basename "$SDKPATH")\\\"\" -arch $ARCH -target ${ARCH}-apple-ios-macabi" \
LDFLAGS="-isysroot $SDKPATH -isystem ${SDKPATH}/System/iOSSupport/usr/include -iframework ${SDKPATH}/System/iOSSupport/System/Library/Frameworks -framework Network -framework Security -framework Foundation -arch $ARCH -target ${ARCH}-apple-ios-macabi" \
./aconfigure --prefix="${PREFIX}" --host="${ARCH}"-apple-darwin_ios $(IFS=' '; echo "${CONFIGURE_EXTRA_PARAMS[*]}") --disable-sdl

make dep && make clean
make VERBOSE=1
make install


create_lib "${PREFIX}/lib"
pkgconfig_ios "${PREFIX}"
popd || exit 1
