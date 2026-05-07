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

# Strips a dump file (removes the first line with the phase name and error messages at the end if they are present)
# for the test suite "srcdumper"
#

set -e

if [[ -z "${1}" ]]; then
    echo "Usage: $0 <dump> [<output>]"
    echo "    <dump> is a path to original dump file"
    echo "    <output> is a path to processed file. If not specified, output is written to stdout"
    exit 1
fi

SCRIPT_DIR=$(realpath "$(dirname "${0}")")
ROOT_DIR=${STATIC_ROOT_DIR:-"${SCRIPT_DIR}/../../../.."}

STRIP_DUMP_SCRIPT="${ROOT_DIR}/tests/tests-u-runner-2/runner/extensions/validators/srcdumper/strip_dump.py"

source "${ROOT_DIR}/scripts/python/venv-utils.sh"

activate_venv
set +e

python3 -B "${STRIP_DUMP_SCRIPT}" "$@"
EXIT_CODE=$?

set -e
deactivate_venv

if [[ ${EXIT_CODE} -ne 0 ]]; then
    exit 1
fi
