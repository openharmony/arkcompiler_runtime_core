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

# Copy <test>.lib<N>.new.ets -> <test>.lib<N>.ets for all new lib versions.
# A backup of the old lib is saved as <test>.lib<N>.ets.orig so that the
# matching restore loop in bin-compat-compile-new-libs.sh can put the old
# content back after the new lib has been compiled. This makes repeat runs
# without --force-generate work correctly.
#
# Two naming conventions are supported:
#
#   1. Non-templated tests:
#        gen/test.ets
#        gen/test.libr.ets       gen/test.libr.new.ets
#      BASE   = gen/test
#      suffix = .ets
#      glob   = ${BASE}.libr*.new.ets
#
#   2. Templated tests:
#        gen/test_0.ets,                        gen/test_1.ets, ...
#        gen/test.libr_0.ets,                   gen/test.libr_1.ets, ...
#        gen/test.libr.new_0.ets,               gen/test.libr.new_1.ets, ...
#      BASE   = gen/test                      BASE   = gen/test
#      suffix = _0.ets                        suffix = _1.ets
#      glob   = ${BASE}.libr*.new_0.ets       glob   = ${BASE}.libr*.new_1.ets
#
# In both cases the old lib is: ${new_lib%.new${suffix}}${suffix}
#
# Usage: bin-compat-copy-new-libs.sh <gen-dir>/<test-id-name>

set -euo pipefail

BASE="$1"

suffix=".ets"

# Only treat as templated test if the BASE stem itself ends with _<N> AND
# there is at least one .new_<N>.ets file in the directory.  This avoids
# false positives on test names that happen to end with a number (e.g.
# remove_enum_constants_2) but are NOT generated from a template.
if [[ "$BASE" =~ ^(.+)_([0-9]+)$ ]]; then
    candidate_suffix="_${BASH_REMATCH[2]}.ets"
    # Check if any .new_<N>.ets file exists (templated pattern)
    for f in "${BASH_REMATCH[1]}".libr*.new${candidate_suffix}; do
        if [ -f "$f" ]; then
            BASE="${BASH_REMATCH[1]}"
            suffix="${candidate_suffix}"
            break
        fi
    done
fi

for new_lib in ${BASE}.libr*.new${suffix}; do
    [ -f "$new_lib" ] || continue
    old_lib="${new_lib%.new${suffix}}${suffix}"
    cp -v "$old_lib" "${old_lib}.orig"
    cp -v "$new_lib" "$old_lib"
done
