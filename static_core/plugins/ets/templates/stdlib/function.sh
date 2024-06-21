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

set -eu -o pipefail

readonly SCRIPT_DIR="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"
readonly GENPATH="${1:-"${SCRIPT_DIR}/../../stdlib"}/std/core"
readonly ROOT_DIR=${STATIC_ROOT_DIR:-"${SCRIPT_DIR}/../../../.."}

source "${ROOT_DIR}/scripts/python/venv-utils.sh"
activate_venv
mkdir -p "${GENPATH}"

readonly FUNC="$GENPATH/Function.ets"
echo "Generating ${FUNC}"
jinja2 "${SCRIPT_DIR}/Function.ets.j2" -o "${FUNC}"

deactivate_venv
