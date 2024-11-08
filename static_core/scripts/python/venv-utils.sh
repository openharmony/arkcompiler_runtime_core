#!/usr/bin/env bash
# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

MY_USERNAME=${SUDO_USER}
if [[ -z "${VENV_DIR}" && -n "${MY_USERNAME}" ]]; then
    MY_HOME=$(grep "^${MY_USERNAME}:" /etc/passwd | cut -d: -f6)
    if [[ ! -e "${MY_HOME}" ]]; then
        MY_HOME=/home/${MY_USERNAME}
    fi
    VENV_DIR=${MY_HOME}/.venv-panda
elif [[ -z "${VENV_DIR}" && -z "${MY_USERNAME}" ]]; then
    VENV_DIR=/root/.venv-panda
fi

function activate_venv()
{
    if [[ -d "${VENV_DIR}" ]]; then
        source "${VENV_DIR}/bin/activate"
        echo "${VENV_DIR} activated"
    fi
}

function deactivate_venv()
{
    if [[ -d "${VENV_DIR}" && -n "${VIRTUAL_ENV}" ]]; then
        deactivate
    fi
}
