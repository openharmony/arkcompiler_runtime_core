#!/bin/bash

#
# Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#

set -e

SCRIPT=$(readlink -e "$BASH_SOURCE")
SCRIPT_DIR=$(dirname "$SCRIPT")

# This script installs all dependencies for formatChecker 
# by creating a virtualenv

# Checking Python version
function version() {
    echo "$@" | awk -F. '{ printf("%d%03d%03d%03d\n", $1,$2,$3,$4); }'
}

MIN_PYTHON_VERSION="3.8.0"
CURRENT_PYTHON_VERSION=$(python3 -c "import platform; print(platform. python_version())")

if [ $(version "$CURRENT_PYTHON_VERSION") -lt $(version $MIN_PYTHON_VERSION) ]; then
    echo "Python version must be greater than $MIN_PYTHON_VERSION"
    exit
fi

# Cheking venv existance
sudo apt install python3-venv -yyy

pushd "${SCRIPT_DIR}/../formatChecker"

python3 -m venv .venv
source .venv/bin/activate

python3 -m ensurepip --upgrade
python3 -m pip install -r requirements.txt

popd
