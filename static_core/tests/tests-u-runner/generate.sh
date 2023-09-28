#!/bin/bash

if [[ -z $1 ]]; then
    echo "Usage: generate.sh <panda source> <generator options>"
    echo "    <panda source> path where the panda source are located"
    echo "    <generator options> options to main.py. To see full list use --help"

    exit 1
fi

ROOT_DIR=$(realpath $1)
shift 1
export PYTHONPATH=$PYTHONPATH:${ROOT_DIR}/tests/tests-u-runner
GENERATOR=${ROOT_DIR}/tests/tests-u-runner/runner/plugins/ets/preparation_step.py
GENERATOR_OPTIONS=$*

source ${ROOT_DIR}/scripts/python/venv-utils.sh
activate_venv
set +e

python3 -B ${GENERATOR} ${GENERATOR_OPTIONS}
EXIT_CODE=$?

set -e
deactivate_venv

exit ${EXIT_CODE}
