#!/bin/bash

set -eu -o pipefail

SCRIPT_DIR="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"
readonly SCRIPT_DIR
readonly GENPATH="${SCRIPT_DIR}/../../stdlib/escompat"

mkdir -p "${GENPATH}"

echo "Generating DataView..."
jinja2 "${SCRIPT_DIR}/DataView.ets.j2" > "$GENPATH/DataView.ets"

echo "Generating Unsigned arrays..."
jinja2 "${SCRIPT_DIR}/typedUArray.ets.j2" > "$GENPATH/TypedUArrays.ets"
echo "Generating Unsigned arrays..."
jinja2 "${SCRIPT_DIR}/typedArray.ets.j2" > "$GENPATH/TypedArrays.ets"

echo "Done."
