#!/bin/bash
# Oliver Epper <oliver.epper@gmail.com>

source "$(dirname "$0")"/build-pjproject-base.sh

PREFIX="${PREFIX}/x86"
rm -rf "${PREFIX}"

pushd pjproject || exit 1

OPUS=("$(pwd)"/../installed/opus/x86)
OPUS_LATEST=${OPUS[${#OPUS[@]} - 1]}
if [[ -d "${OPUS_LATEST}" ]]
then
    CONFIGURE_EXTRA_PARAMS+=("--with-opus=${OPUS_LATEST}")
fi

OPENSSL=("$(pwd)"/../installed/openssl/x86)
OPENSSL_LATEST=${OPENSSL[${#OPENSSL[@]} - 1]}
if [[ -d "${OPENSSL_LATEST}" ]]
then
    CONFIGURE_EXTRA_PARAMS+=("--with-ssl=${OPENSSL_LATEST}")
fi

ANDROID_NDK_ROOT="${NDK}" \
TARGET_ABI=x86 \
./configure-android --use-ndk-cflags --prefix="${PREFIX}" "${CONFIGURE_EXTRA_PARAMS[@]}"

make dep && make clean
make

	pushd pjsip-apps/src/swig || exit 1

	make clean && make

	echo "${PREFIX}"
	exit 1

	mkdir -p "${PREFIX}"
	cp -R java/android/pjsua2/src/main/java "${PREFIX}"
	cp -R java/android/pjsua2/src/main/jniLibs "${PREFIX}"

	popd || exit 1

popd || exit 1