#!/bin/bash

if [[ -z $1 ]]; then
    echo "Usage: runner.sh <panda source> <runner options>"
    echo "    <panda source> path where the panda source are located"
    echo "    <runner options> options to main.py. To see full list use --help"

    exit 1
fi

ROOT_DIR=$(realpath $1)
shift 1

RUNNER="${ROOT_DIR}/tests/tests-u-runner/main.py"

source ${ROOT_DIR}/scripts/python/venv-utils.sh
activate_venv
set +e

echo "${RUNNER_OPTIONS}"

python3 -B "${RUNNER}" "$@"
EXIT_CODE=$?

set -e
deactivate_venv

exit ${EXIT_CODE}
