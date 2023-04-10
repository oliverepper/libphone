#!/bin/bash
# Oliver Epper <oliver.epper@gmail.com>

source "$(dirname "$0")"/build-pjproject-darwin-base.sh

ARM64="${PREFIX}/macos-arm64"
X86_64="${PREFIX}/macos-x86_64"
UNIVERSAL="${PREFIX}/macos-universal"

rm -rf "${UNIVERSAL}"
mkdir "${UNIVERSAL}"

cp -a "${ARM64}"/* "${UNIVERSAL}"

rm -rf "${UNIVERSAL:?}"/lib/*

lipo -create \
    "${ARM64}/lib/libpjproject.a" \
    "${X86_64}/lib/libpjproject.a" \
    -o "${UNIVERSAL}/lib/libpjproject.a"

pkgconfig_macos "${UNIVERSAL}"

