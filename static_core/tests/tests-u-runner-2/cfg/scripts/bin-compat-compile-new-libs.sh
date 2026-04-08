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

# Recompile all libs that had .new.ets replacements, then restore the
# original old-lib content from the .orig backups created by
# bin-compat-copy-new-libs.sh. The recompiled .abc files in
# intermediate/ keep the new-lib bytecode, while the gen/ source files
# revert to their old-lib content so a repeat run without
# --force-generate can start from a clean baseline.
#
# Two naming conventions are supported:
#
#   1. Non-templated tests:
#        BASE_GEN          = gen/test
#        BASE_INTERMEDIATE = intermediate/test
#        suffix = .ets
#        new-lib glob = ${BASE_GEN}.libr*.new.ets
#
#   2. Templated tests:
#        BASE_GEN          = gen/test
#        BASE_INTERMEDIATE = intermediate/test
#        suffix = _0.ets
#        new-lib glob = ${BASE_GEN}.libr*.new_0.ets
#
# In both cases the old lib is: ${new_lib%.new${suffix}}${suffix}
# The output ABC path swaps the gen base for the intermediate base.
#
# Usage: bin-compat-compile-new-libs.sh <es2panda> <arktsconfig> <extension>
#        <opt-level> <gen-prefix> <intermediate-prefix>

set -euo pipefail

ES2PANDA="$1"
ARKTSCONFIG="$2"
EXTENSION="$3"
OPT_LEVEL="$4"
BASE_GEN="$5"
BASE_INTERMEDIATE="$6"

suffix=".ets"

if [[ "$BASE_GEN" =~ ^(.+)_([0-9]+)$ ]]; then
    BASE_GEN="${BASH_REMATCH[1]}"
    suffix="_${BASH_REMATCH[2]}.ets"
    BASE_INTERMEDIATE="${6%_${BASH_REMATCH[2]}}"
fi

for new_lib in ${BASE_GEN}.libr*.new${suffix}; do
    [ -f "$new_lib" ] || continue
    lib_file="${new_lib%.new${suffix}}${suffix}"
    output_abc="${BASE_INTERMEDIATE}${lib_file#${BASE_GEN}}.abc"

    "$ES2PANDA" \
        "--arktsconfig=${ARKTSCONFIG}" \
        "--gen-stdlib=false" \
        "--extension=${EXTENSION}" \
        "--opt-level=${OPT_LEVEL}" \
        "--output=${output_abc}" \
        "$lib_file"
done

# Restore original library files from .orig backups.
for orig_lib in ${BASE_GEN}.libr*${suffix}.orig; do
    [ -f "$orig_lib" ] || continue
    mv "$orig_lib" "${orig_lib%.orig}"
done
