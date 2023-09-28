#!/bin/bash -ex

if [[ -z $1 ]]; then
    echo "Usage: generator.sh <panda source> <generator options>"
    echo "    <panda source> path where the panda source are located"
    echo "    <generator options> options to generator.py. To see full list use --help"

    exit 1
fi

ROOT_DIR=$1
shift 1

GENERATOR=${ROOT_DIR}/plugins/ets/tests/scripts/CtsTools/ets-generator/generator.py
GENERATOR_OPTIONS=$*

source ${ROOT_DIR}/scripts/python/venv-utils.sh
activate_venv

python3 -B ${GENERATOR} ${GENERATOR_OPTIONS}

deactivate_venv
