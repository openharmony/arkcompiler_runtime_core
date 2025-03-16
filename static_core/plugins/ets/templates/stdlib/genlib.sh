#!/usr/bin/env bash
# Copyright (c) 2024 Huawei Device Co., Ltd.
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

set -e -o pipefail

readonly SCRIPT_DIR="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"
readonly GENPATH="${SCRIPT_DIR}/../../stdlib"
readonly GEN_ESCOMPAT_PATH="${1:-${GENPATH}}/escompat"
readonly GEN_STDCORE_PATH="${1:-${GENPATH}}/std/core"
readonly VENV_DIR=${VENV_DIR:-$(realpath ~/.venv-panda)}
readonly ARR="${GEN_ESCOMPAT_PATH}/Array.sts"
readonly BLT_ARR="${GEN_STDCORE_PATH}/BuiltinArray.sts"
readonly BLT_ARR_SORT="${GEN_STDCORE_PATH}/BuiltinArraySort.sts"
readonly BLT_ARR_ARG="${GEN_STDCORE_PATH}/BuiltinArrayAlgorithms.sts"
readonly DATAVIEW="${GEN_ESCOMPAT_PATH}/DataView.sts"
readonly TYPED_ARR="${GEN_ESCOMPAT_PATH}/TypedArrays.sts"
readonly TYPED_UARR="${GEN_ESCOMPAT_PATH}/TypedUArrays.sts"
readonly FUNC="${GEN_STDCORE_PATH}/Function.sts"

cd "$SCRIPT_DIR"

JINJA_PATH="${VENV_DIR}/bin/jinja2"
if ! [[ -f "${JINJA_PATH}" ]]; then
    JINJA_PATH="jinja2"
fi

function format_file() {
    sed -e 's/\b \s\+\b/ /g' | sed -e 's/\s\+$//g' | sed -e 's+/\*\s*\*/\s*++g' | cat -s
}

mkdir -p "${GEN_ESCOMPAT_PATH}"
mkdir -p "${GEN_STDCORE_PATH}"

# Generate Array
echo "Generating ${ARR}"
erb Array_escompat.erb | format_file > "${ARR}"

echo "Generating ${BLT_ARR}"
erb Array_builtin.erb | format_file > "${BLT_ARR}"

echo "Generating ${BLT_ARR_SORT}"
"${JINJA_PATH}" Array_builtin_sort.sts.j2 | format_file > "${BLT_ARR_SORT}"

echo "Generating ${BLT_ARR_ARG}"
"${JINJA_PATH}" Array_builtin_algorithms.sts.j2 | format_file > "${BLT_ARR_ARG}"

# Generate TypedArrays
echo "Generating ${DATAVIEW}"
"${JINJA_PATH}" "${SCRIPT_DIR}/DataView.sts.j2" -o "${DATAVIEW}"

echo "Generating ${TYPED_ARR}"
"${JINJA_PATH}" "${SCRIPT_DIR}/typedArray.sts.j2" -o "${TYPED_ARR}"

echo "Generating ${TYPED_UARR}"
"${JINJA_PATH}" "${SCRIPT_DIR}/typedUArray.sts.j2" -o "${TYPED_UARR}"

# Generate Functions
echo "Generating ${FUNC}"
"${JINJA_PATH}" "${SCRIPT_DIR}/Function.sts.j2" -o "${FUNC}"

exit 0
