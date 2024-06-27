#!/bin/bash
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

set -e -o pipefail

readonly SCRIPT_DIR="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"
readonly GENPATH="${1:-"${SCRIPT_DIR}/../../stdlib"}/escompat"
readonly VENV_DIR=${VENV_DIR:-$(realpath ~/.venv-panda)}
JINJA_PATH="${VENV_DIR}/bin/jinja2"
if ! [[ -f "${JINJA_PATH}" ]]; then
    JINJA_PATH="jinja2"
fi

mkdir -p "${GENPATH}"

echo "Generating $GENPATH/DataView.ets"
"${JINJA_PATH}" "${SCRIPT_DIR}/DataView.ets.j2" -o "$GENPATH/DataView.ets"

echo "Generating $GENPATH/TypedArrays.ets"
"${JINJA_PATH}" "${SCRIPT_DIR}/typedArray.ets.j2" -o "$GENPATH/TypedArrays.ets"

echo "Generating $GENPATH/TypedUArrays.ets"
"${JINJA_PATH}" "${SCRIPT_DIR}/typedUArray.ets.j2" -o "$GENPATH/TypedUArrays.ets"

exit 0
