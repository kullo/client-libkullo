#!/bin/bash
set -o errexit -o nounset -o pipefail
command -v shellcheck > /dev/null && (shellcheck -x "$0" || shellcheck "$0")

if [[ $# -ne 1 ]]; then
    echo "Usage: $(basename "$0") JSONCPP_SOURCE_DIR"
    echo "JSONCPP_SOURCE_DIR is the path to the JsonCpp source code root dir."
    exit 1
fi

SRC_DIR="$1"
AMALGAMATE="$SRC_DIR/amalgamate.py"
if [[ ! -f $AMALGAMATE ]]; then
    echo "Couldn't find $AMALGAMATE"
    exit 1
fi

DEST_DIR="$PWD"/"$(dirname "$0")"
(cd "$SRC_DIR" && python "$AMALGAMATE" -s "$DEST_DIR"/jsoncpp.cpp -i jsoncpp.h)

