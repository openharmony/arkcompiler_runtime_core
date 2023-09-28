#!/bin/bash

VENV_DIR=${VENV_DIR:-$(realpath ~/.venv-panda)}

function activate_venv
{
    if [[ -d ${VENV_DIR} ]]; then
        source ${VENV_DIR}/bin/activate
        echo ${VENV_DIR} activated
    fi
}

function deactivate_venv
{
    if [[ -d ${VENV_DIR} && -n ${VIRTUAL_ENV} ]]; then
        deactivate
    fi
}
