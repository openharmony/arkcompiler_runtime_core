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

# Compares outputs for the test suite "srcdumper"
# Both dumps should be processed by the "strip-dump.sh" script
#

set -e

if [[ -z "${1}" ]]; then
    echo "Usage: $0 <dump1> <dump2>"
    echo "    <dump1> and <dump2> are paths to compared outputs"
    exit 1
fi

SCRIPT_DIR=$(realpath "$(dirname "${0}")")
ROOT_DIR=${STATIC_ROOT_DIR:-"${SCRIPT_DIR}/../../../.."}

AST_COMPARATOR="${ROOT_DIR}/tests/tests-u-runner-2/runner/extensions/validators/srcdumper/ast_comparator.py"

source "${ROOT_DIR}/scripts/python/venv-utils.sh"

activate_venv
set +e

python3 -B "${AST_COMPARATOR}" "$@"
EXIT_CODE=$?

set -e
deactivate_venv

if [[ ${EXIT_CODE} -ne 0 ]]; then
    exit 1
fi
