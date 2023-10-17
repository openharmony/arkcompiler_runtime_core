#!/bin/bash -ex
# Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
