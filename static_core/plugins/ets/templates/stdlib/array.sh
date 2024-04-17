#!/usr/bin/env bash
# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

set -xeo pipefail

readonly SCRIPT_DIR="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"
cd "$SCRIPT_DIR"
readonly GENPATH="${SCRIPT_DIR}/../../stdlib/"

function format_file() {
    sed -e 's/\b \s\+\b/ /g' | sed -e 's/\s\+$//g' | sed -e 's+/\*\s*\*/\s*++g' | cat -s
}

readonly ARR="${GENPATH}/escompat/Array.ets"
echo "Generating ${ARR}"
erb Array_escompat.erb | format_file > "${ARR}"

readonly BLT_ARR="${GENPATH}/std/core/BuiltinArray.ets"
echo "Generating ${BLT_ARR}"
erb Array_builtin.erb | format_file > "${BLT_ARR}"

readonly BLT_ARR_SORT="${GENPATH}/std/core/BuiltinArraySort.ets"
echo "Generating ${BLT_ARR_SORT}"
jinja2 Array_builtin_sort.ets.j2 | format_file > "${BLT_ARR_SORT}"

readonly BLT_ARR_ARG="${GENPATH}/std/core/BuiltinArrayAlgorithms.ets"
echo "Generating ${BLT_ARR_ARG}"
jinja2 Array_builtin_algorithms.ets.j2 | format_file > "${BLT_ARR_ARG}"
