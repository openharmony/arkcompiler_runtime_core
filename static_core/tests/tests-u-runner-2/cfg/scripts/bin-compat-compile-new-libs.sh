#!/bin/bash
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

# Recompile all libs that had .new.ets replacements.
# Only libs with a .new.ets counterpart are recompiled; others keep their
# existing .abc from the initial compilation.
#
# Usage: bin-compat-compile-new-libs.sh <es2panda> <arktsconfig> <extension>
#        <opt-level> <gen-prefix> <intermediate-prefix>
#
# Arguments:
#   es2panda             - path to es2panda binary
#   arktsconfig          - path to arktsconfig.json
#   extension            - source extension (ets)
#   opt-level            - optimization level (0-2)
#   gen-prefix           - ${work-dir}/gen/${test-id:parent}/${test-id:stem}
#   intermediate-prefix  - ${work-dir}/intermediate/${test-id:parent}/${test-id:stem}

set -e

ES2PANDA="$1"
ARKTSCONFIG="$2"
EXTENSION="$3"
OPT_LEVEL="$4"
GEN_PREFIX="$5"
INTERMEDIATE_PREFIX="$6"

for new_lib in "${GEN_PREFIX}".libr*.new.ets; do
    [ -f "$new_lib" ] || continue

    # Source: .libr.new.ets -> .libr.ets (already overwritten by copy-new-libs)
    lib_file="${new_lib%.new.ets}.ets"

    # Output ABC: swap gen/ prefix for intermediate/, append .abc
    # gen/test.libr.ets -> intermediate/test.libr.ets.abc
    output_abc="${INTERMEDIATE_PREFIX}${lib_file#${GEN_PREFIX}}.abc"

    "$ES2PANDA" \
        "--arktsconfig=${ARKTSCONFIG}" \
        "--gen-stdlib=false" \
        "--extension=${EXTENSION}" \
        "--opt-level=${OPT_LEVEL}" \
        "--output=${output_abc}" \
        "$lib_file"
done

# Restore original library files from .orig backup
for orig_lib in "${GEN_PREFIX}".libr*.ets.orig; do
    [ -f "$orig_lib" ] || continue
    lib_file="${orig_lib%.orig}"
    cp "$orig_lib" "$lib_file"
    rm "$orig_lib"
done
