#!/bin/bash
set -o errexit -o nounset -o pipefail
which shellcheck > /dev/null && shellcheck "$0"

OUTPUT_FILE="$1"

SED=sed
if [[ "$(uname)" == "Darwin" ]]; then
	SED=gsed
	if ! which "$SED" > /dev/null; then
		echo -e "We need GNU sed. BSD sed as shipped by OS X isn't supported. Install:"
		echo -e "\tbrew install gnu-sed"
		exit 1
	fi
fi

tests=($( \
	grep --recursive --no-filename --null "^[[:space:]]*K_TEST\(_F\)\?" . | \
	"$SED" "s/[[:space:]]*K_TEST\(_F\)\?.\([[:alnum:]_]\+\), \([[:alnum:]_]\+\).*/\2_\3/g" \
))

rm "$OUTPUT_FILE" || true >/dev/null 2>&1

for test_name in "${tests[@]}"; do
	echo "extern int PULL_IN_$test_name;" >> "$OUTPUT_FILE"
done
echo >> "$OUTPUT_FILE"
echo "void pullInTests() {" >> "$OUTPUT_FILE"
for test_name in "${tests[@]}"; do
	echo -e "\tPULL_IN_$test_name = 0;" >> "$OUTPUT_FILE"
done
echo "}" >> "$OUTPUT_FILE"

