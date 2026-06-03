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
#
# Per-test wrapper invoked by cfg/workflows/panda-metadata-parity.yaml.
# Activates the project Python venv (so PyYAML is available) and runs
# check-metadata-parity.py on the (LHS, RHS) pair passed by the workflow.

set -e

if [[ -z "${1}" ]]; then
    echo "Usage: $0 <metadata-source-file>" >&2
    echo "  The binary-metadata counterpart path is derived by swapping the" >&2
    echo "  'metadata' segment in the given path with 'binary-metadata'." >&2
    exit 1
fi

SCRIPT_DIR=$(realpath "$(dirname "${0}")")
ROOT_DIR=${STATIC_ROOT_DIR:-"${SCRIPT_DIR}/../../../.."}

PARITY_CHECKER="${SCRIPT_DIR}/check_metadata_parity.py"

source "${ROOT_DIR}/scripts/python/venv-utils.sh"

activate_venv
set +e

python3 -B "${PARITY_CHECKER}" "$@"
EXIT_CODE=$?

set -e
deactivate_venv

if [[ ${EXIT_CODE} -ne 0 ]]; then
    exit 1
fi
