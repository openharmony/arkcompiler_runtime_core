#!/bin/bash
# Copyright (c) 2025 Huawei Device Co., Ltd.
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

# Utility function to include in each script.
# Example: source "$(dirname "$0")/common_lib.sh"

init_shell_options() {
    # Sane defaults.
    set -eu
    set -o pipefail
}

# Chdir to project root
chdir_root() {
    cd "$(dirname "$0")/.."
}

init_py_env() {
    init_shell_options
    chdir_root
    cd compiler
    source .venv/bin/activate
    set -x
}
