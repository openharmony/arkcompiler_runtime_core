#!/bin/bash -e
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

# check-venv.sh: Check if all packages from requirements file are present in current active environment
# Usage: check-venv.sh [-v] <requirements_file>
#

# Check if Python is available
if ! command -v python3 &> /dev/null; then
    print_error "Python3 is not available"
    exit 1
fi

# Check if pip is available
if ! python3 -m pip --version &> /dev/null; then
    print_error "pip is not available"
    exit 1
fi

CHECK_VENV_DIR=$(realpath "$(dirname "${0}")")
CHECK_VENV_PY=${CHECK_VENV_DIR}/check_venv.py

python3 -B "${CHECK_VENV_PY}" "$@"
