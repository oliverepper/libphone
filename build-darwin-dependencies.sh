#!/usr/bin/env bash

## CODECS
CONFIGURE_EXTRA_PARAMS="--disable-opencore-amr --disable-bcg729 --enable-g711-codec \
    --enable-g722-codec --disable-g7221-codec --disable-gsm-codec --without-external-gsm \
    --disable-ilbc-codec --disable-l16-codec --disable-silk --disable-speex-codec \
    --without-external-speex --disable-speex-aec"

## FEATURES
CONFIGURE_EXTRA_PARAMS+=" --disable-ffmpeg --enable-pjsua2 --enable-sound \
    --enable-libsrtp --disable-upnp --disable-v4l2 --disable-video \
    --disable-vpx --disable-libyuv"

mkdir -p build
pushd build || exit 1
sh ../scripts/get-ios-toolchain.sh &&
sh ../scripts/build-opus-darwin.sh "$(pwd)/../installed/opus" &&

CONFIGURE_EXTRA_PARAMS="${CONFIGURE_EXTRA_PARAMS[@]}" sh ../scripts/build-pjproject-ios-arm64.sh "$(pwd)/../installed" &&

CONFIGURE_EXTRA_PARAMS="${CONFIGURE_EXTRA_PARAMS[@]}" sh ../scripts/build-pjproject-simulator-arm64.sh "$(pwd)/../installed" &&
CONFIGURE_EXTRA_PARAMS="${CONFIGURE_EXTRA_PARAMS[@]}" sh ../scripts/build-pjproject-simulator-x86_64.sh "$(pwd)/../installed" &&

CONFIGURE_EXTRA_PARAMS="${CONFIGURE_EXTRA_PARAMS[@]}" sh ../scripts/build-pjproject-macos-arm64.sh "$(pwd)/../installed" &&
CONFIGURE_EXTRA_PARAMS="${CONFIGURE_EXTRA_PARAMS[@]}" sh ../scripts/build-pjproject-macos-x86_64.sh "$(pwd)/../installed" &&
CONFIGURE_EXTRA_PARAMS="${CONFIGURE_EXTRA_PARAMS[@]}" sh ../scripts/create-pjproject-macos-universal.sh "$(pwd)/../installed" &&

CONFIGURE_EXTRA_PARAMS="${CONFIGURE_EXTRA_PARAMS[@]}" sh ../scripts/build-pjproject-catalyst-arm64.sh "$(pwd)/../installed" &&
CONFIGURE_EXTRA_PARAMS="${CONFIGURE_EXTRA_PARAMS[@]}" sh ../scripts/build-pjproject-catalyst-x86_64.sh "$(pwd)/../installed" &&

echo "done"

popd build || exit 1
