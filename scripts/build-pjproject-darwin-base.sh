#!/bin/bash
# Oliver Epper <oliver.epper@gmail.com>

set -e
env -i

if [ $# -eq 0 ]
then
    echo "sh $(basename "$0") <absolute path>"
    exit 1
fi

export PREFIX="$1/pjproject"
PJPROJECT_URL=https://github.com/pjsip/pjproject
PJPROJECT_VERSION=2.14
# PJPROJECT_COMMIT=master

if [ -d pjproject ]
then
    pushd pjproject
    git reset --hard "${PJPROJECT_VERSION}"
    # git pull https://github.com/pjsip/pjproject.git
    # git reset --hard "${PJPROJECT_COMMIT}"
    popd
else
    git -c advice.detachedHead=false clone --depth 1 --branch "${PJPROJECT_VERSION}" "${PJPROJECT_URL}"
    # git clone https://github.com/pjsip/pjproject.git
    # git -c advice.detachedHead=false -C pjproject checkout ${PJPROJECT_COMMIT}    
fi

# create base configuration for pjproject build
pushd pjproject
cat << EOF > pjlib/include/pj/config_site.h
#define PJ_HAS_SSL_SOCK 1
#undef PJ_SSL_SOCK_IMP
#define PJ_SSL_SOCK_IMP PJ_SSL_SOCK_IMP_APPLE
#include <pj/config_site_sample.h>
EOF
popd

function prepare {
    local WANTS_IPHONE=$1
    local WANTS_VIDEO=$2

    git reset --hard
    git clean -fxd

    cat << EOF > pjlib/include/pj/config_site.h
#define PJ_HAS_SSL_SOCK 1
#undef PJ_SSL_SOCK_IMP
#define PJ_SSL_SOCK_IMP PJ_SSL_SOCK_IMP_APPLE
#include <pj/config_site_sample.h>
EOF

    if [[ "${WANTS_IPHONE}" = "YES" ]]; then
        echo "🔧 adding iPhone support"
        sed -i '' -e '1i\
#define PJ_CONFIG_IPHONE 1
' pjlib/include/pj/config_site.h
    fi

    if [[ "${WANTS_VIDEO}" = "YES" ]]; then
        echo "🔧 adding video support"
        sed -i '' -e '1i\
#define PJMEDIA_HAS_VIDEO 1 \
#define PJMEDIA_HAS_VID_TOOLBOX_CODEC 1
' pjlib/include/pj/config_site.h
    fi
}

function create_lib {
    pushd "$1"
    EXTRA_LIBS=()
    if [ -d "${OPUS_LATEST}" ]; then
        EXTRA_LIBS+=("${OPUS_LATEST}/lib/libopus.a")
        unset OPUS
        unset OPUS_LATEST
    fi
    # if [[ -d "${SDL_LATEST}" ]]; then
    #     EXTRA_LIBS+=("${SDL_LATEST}/lib/libSDL2.a")
    #     unset SDL
    #     unset SDL_LATEST
    # fi

    LLVM=(/opt/homebrew/Cellar/llvm/*)
    LLVM_LATEST=${LLVM[${#LLVM[@]} - 1]}
    if [[ -d "${LLVM_LATEST}" ]]
    then
        "${LLVM_LATEST}"/bin/llvm-libtool-darwin -static -o libpjproject.a ./*.a "${EXTRA_LIBS[@]}"
        touch libpjproject_is_sane
    else
        libtool -static -o libpjproject.a ./*.a "${EXTRA_LIBS[@]}"
        touch libpjproject_is_broken
    fi
    popd
}

function pkgconfig_ios {
    BASEPATH="$1"
    PCPATH="${BASEPATH}/lib/pkgconfig"
    mkdir -p "${PCPATH}"
    PCFILE="${BASEPATH}/lib/pkgconfig/libpjproject.pc"
    cat << 'EOF' > "${PCFILE}"
Name: libpjproject
Description: Multimedia communication library
URL: http://www.pjsip.org
EOF

    cat << EOF >> "${PCFILE}"
Version: ${PJPROJECT_VERSION}

Libs: -L$(realpath "${BASEPATH}"/lib) -lpjproject -framework CFNetwork -framework UIKit -framework Foundation -framework Security -framework Network -framework AVFoundation -framework CoreMedia -framework CoreAudio -framework CoreVideo -framework AudioToolbox -framework VideoToolbox
Cflags: -I$(realpath "${BASEPATH}"/include) -DPJ_AUTOCONF=1  -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1
EOF
}

function pkgconfig_macos {
    BASEPATH="$1"
    PCPATH="${BASEPATH}/lib/pkgconfig"
    mkdir -p "${PCPATH}"
    PCFILE="${BASEPATH}/lib/pkgconfig/libpjproject.pc"
    cat << 'EOF' > "${PCFILE}"
Name: libpjproject
Description: Multimedia communication library
URL: http://www.pjsip.org
EOF

cat << EOF >> "${PCFILE}"
Version: ${PJPROJECT_VERSION}

Libs: -L$(realpath "${BASEPATH}"/lib) -lpjproject -framework Carbon -framework AppKit -framework Security -framework Network -framework AVFoundation -framework CoreMedia -framework CoreAudio -framework CoreVideo -framework AudioToolbox -framework VideoToolbox -framework Metal -framework IOKit
Cflags: -I$(realpath "${BASEPATH}"/include) -DPJ_AUTOCONF=1  -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1
EOF
}
