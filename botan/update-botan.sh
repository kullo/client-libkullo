#!/bin/bash
set -o errexit -o nounset -o pipefail
which shellcheck > /dev/null && shellcheck "$0"

# not used at the moment:
# aes_ni,aes_ssse3,sha1_sse2
MODS_AES="aes"
MODS_SHA="sha1,sha2_32,sha2_64"
MODS_COMMON="auto_rng,codec_filt,eme_oaep,emsa_pssr,gcm,hres_timer,rsa,srp6,system_rng"
MODS_UNIX="${MODS_AES},${MODS_SHA},${MODS_COMMON},dev_random,proc_walk"
MODS_IOS="${MODS_AES},${MODS_SHA},${MODS_COMMON},darwin_secrandom"
MODS_WINDOWS="${MODS_AES},${MODS_SHA},${MODS_COMMON},cryptoapi_rng,win32_stats"

DO_DOWNLOAD=0
DO_UPDATE=0

if [[ $# -eq 1 ]]; then
	DO_DOWNLOAD=1
	DO_UPDATE=1
	VERSION="$1"

elif [[ $# -eq 2  ]]; then
	if [[ "$1" == "--download" ]]; then
		DO_DOWNLOAD=1
		VERSION="$2"

	elif [[ "$1" == "--local" ]]; then
		DO_UPDATE=1
		SRC_DIR="$2"
	fi

else
    echo -e "Usage:"
    echo -e "\t$(basename "$0") [--download] BOTAN_VERSION"
    echo -e "\t$(basename "$0") --local DIR"
    exit 1
fi


if [[ "$DO_DOWNLOAD" -eq 1 ]]; then
	SRC_DIR="/tmp/update-botan/$VERSION"
	(
		mkdir -p "$SRC_DIR" && \
		cd "$SRC_DIR" && \
		(rm -r ./* || true) && \
		wget "http://botan.randombit.net/releases/Botan-$VERSION.tgz" && \
		wget "http://botan.randombit.net/releases/Botan-$VERSION.tgz.asc" && \
		gpg --verify "Botan-$VERSION.tgz.asc" "Botan-$VERSION.tgz" && \
		tar -xf "Botan-$VERSION.tgz"
	)
	SRC_DIR="$SRC_DIR/Botan-$VERSION"
	echo "Downloaded Botan to $SRC_DIR"
fi


if [[ "$DO_UPDATE" -eq 1 ]]; then
	CONFIGURE="$SRC_DIR/configure.py"
	if [[ ! -f $CONFIGURE ]]; then
		echo "Couldn't find $CONFIGURE"
		exit 1
	fi

	function gen_amalgamation {
		if [[ $# -lt 2 ]]; then
		    echo "Usage: gen_amalgamation DEST_DIR CONFIGURE_ARGS"
		    exit 1
		fi
		DEST_DIR="$PWD"/"$(dirname "$0")"/"$1"
		shift

		(cd "$SRC_DIR" && \
		"$CONFIGURE" --via-amalgamation --no-autoload --disable-shared --with-boost --with-zlib "$@")
		mkdir -p "$DEST_DIR"
		mv "$SRC_DIR"/botan_all.cpp        "$DEST_DIR/"
		mv "$SRC_DIR"/botan_all.h          "$DEST_DIR/"
		mv "$SRC_DIR"/botan_all_internal.h "$DEST_DIR/"
	}

	gen_amalgamation android-x86 "--enable-modules=${MODS_UNIX}" --os=android --cpu=x86_32 --cc=clang
	gen_amalgamation android-arm "--enable-modules=${MODS_UNIX}" --os=android --cpu=armv7-a --cc=clang
	gen_amalgamation linux32     "--enable-modules=${MODS_UNIX}" --os=linux --cpu=x86_32 --cc=gcc
	gen_amalgamation linux64     "--enable-modules=${MODS_UNIX}" --os=linux --cpu=x86_64 --cc=gcc
	gen_amalgamation osx         "--enable-modules=${MODS_UNIX}" --os=darwin --cpu=x86_64 --cc=clang
	gen_amalgamation ios-x86     "--enable-modules=${MODS_IOS}"  --os=darwin --cpu=x86_32 --cc=clang
	gen_amalgamation ios-x86_64  "--enable-modules=${MODS_IOS}"  --os=darwin --cpu=x86_64 --cc=clang
	gen_amalgamation ios-armv7   "--enable-modules=${MODS_IOS}"  --os=darwin --cpu=armv7-a --cc=clang
	gen_amalgamation ios-armv8   "--enable-modules=${MODS_IOS}"  --os=darwin --cpu=armv8-a --cc=clang
	gen_amalgamation win32-msvc  "--enable-modules=${MODS_WINDOWS}" --os=windows --cpu=x86_32 --cc=msvc
fi
