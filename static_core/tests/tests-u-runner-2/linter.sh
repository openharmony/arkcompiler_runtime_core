#!/bin/bash
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

set -e

declare -i EXIT_CODE=0
SCRIPT_DIR=$(realpath "$(dirname "${0}")")
ROOT_DIR=${STATIC_ROOT_DIR:-"${SCRIPT_DIR}/../.."}
RUNNER_DIR=${ROOT_DIR}/tests/tests-u-runner-2
YAML_LINTER_DIR=${ROOT_DIR}/tests/test-linters

function run() {
    local desc=$1 && shift
    local executor=$1 && shift
    local args=("$@")
    echo "...Run ${desc}"
    if ! $executor "${args[@]}"; then
        ((EXIT_CODE++))
    fi
}

source "${ROOT_DIR}/scripts/python/venv-utils.sh"
activate_venv

set +e

cd "${RUNNER_DIR}" || exit 1

run "Pylint /runner, main.py" pylint --rcfile .pylintrc runner main.py --ignore=test
# Disable duplicate code and protected access for tests
run "Pylint on tests" pylint --rcfile .pylintrc runner/test/ --disable=protected-access --disable=duplicate-code

run "MyPy main.py" mypy main.py
run "MyPy /runner" mypy -p runner
run "Ruff check" ruff check .
run "Check complexity with radon" flake8 --radon-max-cc 15 --select=R701 .
run "Yamllint check" python "${YAML_LINTER_DIR}/check_yaml_templates.py" ./cfg

set -e

deactivate_venv

if [[ "${EXIT_CODE}" != "0" ]]; then
    echo "...Linter checks failed (EXIT_CODE = ${EXIT_CODE})"
    exit 1
else
    echo '...Linter checks passed'
    exit 0
fi
