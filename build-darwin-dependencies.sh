#!/usr/bin/env bash

mkdir -p build
pushd build || exit 1
sh ../scripts/get-ios-toolchain.sh
sh ../scripts/build-opus-darwin.sh "$(pwd)/../installed/opus"
sh ../scripts/build-sdl-darwin.sh "$(pwd)/../installed/sdl"

sh ../scripts/build-pjproject-ios-arm64.sh "$(pwd)/../installed"

sh ../scripts/build-pjproject-simulator-arm64.sh "$(pwd)/../installed"
sh ../scripts/build-pjproject-simulator-x86_64.sh "$(pwd)/../installed"

sh ../scripts/build-pjproject-macos-arm64.sh "$(pwd)/../installed"
sh ../scripts/build-pjproject-macos-x86_64.sh "$(pwd)/../installed"

sh ../scripts/build-pjproject-catalyst-arm64.sh "$(pwd)/../installed"
sh ../scripts/build-pjproject-catalyst-x86_64.sh "$(pwd)/../installed"

popd build || exit 1
