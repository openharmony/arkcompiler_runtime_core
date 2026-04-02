#!/usr/bin/env bash
# Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

function get_my_home()
{
    local user_name=$1
    local my_home
    my_home=$(grep "^${user_name}:" /etc/passwd | cut -d: -f6)
    if [[ ! -e "${my_home}" ]]; then
        my_home=/home/${user_name}
    fi
    echo "${my_home}"
}

DOT_VENV_PANDA=.venv-panda
MY_USERNAME=${SUDO_USER}
if [[ -z "${VENV_DIR}" && -n "${MY_USERNAME}" ]]; then
    my_home=$( get_my_home "${MY_USERNAME}" )
    VENV_DIR=${my_home}/${DOT_VENV_PANDA}
elif [[ -z "${VENV_DIR}" && -z "${MY_USERNAME}" ]]; then
    if [[ -n "${USER}" ]]; then
        my_home=$( get_my_home "${USER}" )
        VENV_DIR=${my_home}/${DOT_VENV_PANDA}
    else
        VENV_DIR=/root/${DOT_VENV_PANDA}
    fi
fi

function activate_venv()
{
    if [[ -d "${VENV_DIR}" ]]; then
        source "${VENV_DIR}/bin/activate"
        echo "${VENV_DIR} activated"

        # tmp fix for CI to install required modules for init pipeline
        if ! python -c "import pydantic" >/dev/null 2>&1; then
            python -m pip install "pydantic==2.12.5" "pydantic-core==2.41.5"
        fi
    fi
}

function deactivate_venv()
{
    if [[ -d "${VENV_DIR}" && -n "${VIRTUAL_ENV}" ]]; then
        deactivate
    fi
}
