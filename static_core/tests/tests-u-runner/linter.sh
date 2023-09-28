#!/bin/bash

if [[ -z $1 ]]; then
    echo "Usage: linter.sh <panda source>"
    echo "    <panda source> path where the panda source are located"
    exit 1
fi

ROOT_DIR=$(realpath $1)


function save_exit_code {
    EXIT_CODE=$(($1 + $2))
}

source ${ROOT_DIR}/scripts/python/venv-utils.sh
activate_venv

set +e

EXIT_CODE=0
RUNNER_DIR=${ROOT_DIR}/tests/tests-u-runner

cd ${RUNNER_DIR}

pylint --rcfile .pylintrc runner main.py
save_exit_code ${EXIT_CODE} $?

mypy main.py
save_exit_code ${EXIT_CODE} $?

mypy -p runner
save_exit_code ${EXIT_CODE} $?

set -e

deactivate_venv
exit ${EXIT_CODE}
