#!/usr/bin/env bash
# Oliver Epper <oliver.epper@gmail.com>

source "$(dirname "$0")"/functions.sh

if [ $# -lt 4 ]; then
	print_error "Usage: $0 <install-dir> <stage-dir> <codesign-identity> <postfix> [include_if_available...]"
	exit 1
fi

set -e

INSTALL_DIR=$1
STAGE_DIR=$2/xcframework
CODESIGN_IDENTITY=$3
POSTFIX=$4
shift 4
INCLUDE=("$@")

INCLUDED_PLATFORMS=("${INCLUDE[@]}")
echo "Including: " "${INCLUDED_PLATFORMS[@]}"

extract_version() {
	local install_dir="${INSTALL_DIR}/${1}"
	local dso
	dso=$(find "${install_dir}/lib" -name "libphone*" -type f | sort -V | tail -n 1)
	local version="${dso##*libphone.}"
	local version="${version%.dylib}"
	echo "${version}"
}

copy_dso() {
	local install_dir="${INSTALL_DIR}/${1}"
	local stage_dir="${STAGE_DIR}/${1}"
	local soversion="${2}"
	mkdir -p "${stage_dir}"
	version=$(extract_version "${1}")
	cp "${install_dir}/lib/libphone.${soversion}.dylib" "${stage_dir}"
}

sign() {
	local stage_dir="${STAGE_DIR}/${1}"
	local soversion="${2}"
	codesign --deep --force --verify --verbose -s "${CODESIGN_IDENTITY}" --options runtime "${stage_dir}/libphone.${soversion}.dylib"
    if [ -d "${INSTALL_DIR}/${1}/PhoneKit.framework" ]; then
	    codesign --deep --force --verify --verbose -s "${CODESIGN_IDENTITY}" --options runtime "${INSTALL_DIR}/${1}/PhoneKit.framework/PhoneKit"
    fi
}

create_debug_symbols() {
	local stage_dir="${STAGE_DIR}/${1}"
	local soversion="${2}"
	dsymutil "${stage_dir}/libphone.${soversion}.dylib"
    if [ -d "${INSTALL_DIR}/${1}/PhoneKit.framework" ]; then
        dsymutil -o "${INSTALL_DIR}/${1}/PhoneKit.framework.dSYM" "${INSTALL_DIR}/${1}/PhoneKit.framework/PhoneKit" 
    fi
}

add_params() {
	local install_dir="${INSTALL_DIR}/${1}"
	local stage_dir="${STAGE_DIR}/${1}"
	local soversion="${2}"
	local stage_dir_realpath
	stage_dir_realpath=$(realpath "${stage_dir}")
    if [ ! -d "${install_dir}/PhoneKit.framework" ]; then
	    PARAMS+=(-library "${stage_dir}/libphone.${soversion}.dylib")
        PARAMS+=(-headers "${install_dir}/include")
        PARAMS+=(-debug-symbols "${stage_dir_realpath}/libphone.${soversion}.dylib.dSYM")
    else
        PARAMS+=(-framework "${install_dir}/PhoneKit.framework")
	    PARAMS+=(-debug-symbols "${install_dir}/PhoneKit.framework.dSYM")
    fi
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

included () {
	local platform=${1}
	for included in "${INCLUDED_PLATFORMS[@]}"; do
		if [ "${platform}" = "${included}" ]; then
			return 0
		fi
	done
	return 1
}

for node in $(find "${INSTALL_DIR}" -type d -depth 1 -not -path "*xcframework"); do
	if [ ! -d "${node}" ]; then
		continue
	fi
	platform=$(basename "${node}")

	if included "${platform}" || [ "${#INCLUDED_PLATFORMS[@]}" -eq 0 ]; then
		version=$(extract_version "${platform}")
		VERSIONS+=("${version}")
		soversion=$(extract_soversion "${version}")
		echo "Processing: ${platform} - ${version}, packing soversion: ${soversion}"

		copy_dso "${platform}" "${soversion}"
		sign "${platform}" "${soversion}"
		create_debug_symbols "${platform}" "${soversion}"
		add_params "${platform}" "${soversion}"
	fi
done

version=$(greatest_version "${VERSIONS[@]}")
if [ -n "${POSTFIX}" ]; then
	filename="${INSTALL_DIR}/libphone-${version}-${POSTFIX}.xcframework"
else
	filename="${INSTALL_DIR}/libphone-${version}.xcframework"
fi

PARAMS+=(-output "${filename}")

rm -rf "${filename}"

xcodebuild -create-xcframework "${PARAMS[@]}"
zip -r "${filename}.zip" "${filename}"
shasum -a 256 "${filename}.zip"
