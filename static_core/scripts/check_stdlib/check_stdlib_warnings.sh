#!/usr/bin/env bash
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

STDLIB_COMPILER=""
OUTPUT=""
ARKTSCONFIG=""
WHITELIST="${SCRIPT_DIR}/whitelist.txt"

function usage() {
    cat <<EOF
Usage: $0 --stdlib-compiler <path> --output <path> --arktsconfig <path> [--whitelist <path>]
EOF
}

function require_value() {
    local option="$1"
    local value="${2:-}"
    if [[ -z "$value" ]]; then
        echo "Error: ${option} requires a value" >&2
        usage >&2
        exit 2
    fi
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --stdlib-compiler)
            require_value "$1" "${2:-}"
            STDLIB_COMPILER="$2"
            shift 2
            ;;
        --stdlib-compiler=*)
            STDLIB_COMPILER="${1#*=}"
            shift
            ;;
        --output)
            require_value "$1" "${2:-}"
            OUTPUT="$2"
            shift 2
            ;;
        --output=*)
            OUTPUT="${1#*=}"
            shift
            ;;
        --arktsconfig)
            require_value "$1" "${2:-}"
            ARKTSCONFIG="$2"
            shift 2
            ;;
        --arktsconfig=*)
            ARKTSCONFIG="${1#*=}"
            shift
            ;;
        --whitelist)
            require_value "$1" "${2:-}"
            WHITELIST="$2"
            shift 2
            ;;
        --whitelist=*)
            WHITELIST="${1#*=}"
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Error: unknown argument: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

if [[ -z "$STDLIB_COMPILER" ]]; then
    echo "Error: stdlib compiler path was not specified" >&2
    usage >&2
    exit 2
fi

if [[ -z "$OUTPUT" ]]; then
    echo "Error: output path was not specified" >&2
    usage >&2
    exit 2
fi

if [[ -z "$ARKTSCONFIG" ]]; then
    echo "Error: arktsconfig path was not specified" >&2
    usage >&2
    exit 2
fi

if [[ ! -x "$STDLIB_COMPILER" ]]; then
    echo "Error: stdlib compiler was not detected or is not executable: $STDLIB_COMPILER" >&2
    exit 2
fi

if [[ ! -f "$ARKTSCONFIG" ]]; then
    echo "Error: arktsconfig was not detected: $ARKTSCONFIG" >&2
    exit 2
fi

mkdir -p "$(dirname "$OUTPUT")"

COMPILER=(
    "$STDLIB_COMPILER"
    --opt-level=2
    --output="$OUTPUT"
    --extension=ets
    --gen-stdlib=true
    --arktsconfig="$ARKTSCONFIG"
)

if [[ ! -f "$WHITELIST" ]]; then
    echo "Error: whitelist was not detected: $WHITELIST" >&2
    exit 2
fi

output_file="$(mktemp)"
whitelist_file="$(mktemp)"
new_failures_file="$(mktemp)"
tmp_got_orig="$(mktemp)"
tmp_got_keys="$(mktemp)"

function cleanup() {
    rm -f "$output_file" "$whitelist_file" "$new_failures_file" "$tmp_got_orig" "$tmp_got_keys"
}
trap cleanup EXIT

set +e
"${COMPILER[@]}" >"$output_file" 2>&1
compiler_status=$?
set -e

function normalize() {
    sed -E 's/:[0-9]+(:[0-9]+)?]/]/'
}

tr -d '\r' <"$WHITELIST" | \
    awk 'NF && $0 !~ /^[[:space:]]*#/' | \
    { grep -E 'Warning W[0-9]+' || true; } | \
    normalize >"$whitelist_file"

tr -d '\r' <"$output_file" | \
    { grep -E 'Warning W[0-9]+' || true; } | \
    { grep -v -E 'Warning W163154:' || true; } >"$tmp_got_orig"

normalize <"$tmp_got_orig" >"$tmp_got_keys"

awk -v wlfile="$whitelist_file" '
    FILENAME == wlfile {
        allowed[$0]++;
        next
    }
    {
        getline original < "'"$tmp_got_orig"'"
        key = $0
        seen[key]++
        if (seen[key] > allowed[key]) {
            print original
        }
    }
' "$whitelist_file" "$tmp_got_keys" >"$new_failures_file"

if [[ -s "$new_failures_file" ]]; then
    echo "Detected new warnings:"
    echo "------------------------------------------------------"
    cat "$new_failures_file"
    echo "------------------------------------------------------"
    echo "Compiler exit code: $compiler_status"
    exit 1
fi

if [[ "$compiler_status" -ne 0 ]]; then
    exit "$compiler_status"
fi
