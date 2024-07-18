#!/usr/bin/env bash
# Oliver Epper <oliver.epper@gmail.com>

source "$(dirname "$0")"/functions.sh

if [ $# -lt 4 ]; then
	print_error "Usage: $0 <install-dir> <stage-dir> <soversion> <codesign-identity> <excludes...>"
	exit 1
fi

set -e

INSTALL_DIR=$1
STAGE_DIR=$2/xcframework
SOVERSION=$3
CODESIGN_IDENTITY=$4
shift 4
EXCLUDES=("$@")

EXCLUDED_PLATFORMS=("libphone.xcframework")
EXCLUDED_PLATFORMS+=("${EXCLUDES[@]}")
echo "Excluding: ${EXCLUDED_PLATFORMS[@]}"

copy_dso() {
	local install_dir="${INSTALL_DIR}/${1}"
	local stage_dir="${STAGE_DIR}/${1}"
	mkdir -p "${stage_dir}"
	cp "${install_dir}"/lib/libphone.${SOVERSION}.dylib "${stage_dir}"
}

sign() {
	local stage_dir="${STAGE_DIR}/${1}"
	codesign --deep --force --verify --verbose -s "${CODESIGN_IDENTITY}" --options runtime "${stage_dir}"/libphone.${SOVERSION}.dylib
}

create_debug_symbols() {
	local stage_dir="${STAGE_DIR}/${1}"
	dsymutil "${stage_dir}"/libphone.${SOVERSION}.dylib
}

add_params() {
	local install_dir="${INSTALL_DIR}/${1}"
	local stage_dir="${STAGE_DIR}/${1}"
	local stage_dir_realpath=$(realpath ${stage_dir})
	PARAMS+=(-library "${stage_dir}/libphone.${SOVERSION}.dylib")
	PARAMS+=(-headers "${install_dir}/include")
	PARAMS+=(-debug-symbols "${stage_dir_realpath}/libphone.${SOVERSION}.dylib.dSYM")
}

excluded () {
	local platform=${1}
	for excluded in "${EXCLUDED_PLATFORMS[@]}"; do
		if [ "${platform}" = "${excluded}" ]; then
			return 0
		fi
	done
	return 1
}

for node in "${INSTALL_DIR}"/*; do
	if [ ! -d "${node}" ]; then
		continue
	fi
	platform=$(basename "${node}")

	if excluded "${platform}"; then
		continue
	fi
	echo "Processing: ${platform}"
	copy_dso "${platform}"
	sign "${platform}"
	create_debug_symbols "${platform}"
	add_params "${platform}"
done

PARAMS+=(-output "${INSTALL_DIR}"/libphone.xcframework)

rm -rf "${INSTALL_DIR}"/libphone.xcframework

xcodebuild -create-xcframework "${PARAMS[@]}"