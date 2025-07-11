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

# Logging
COLOR_RST='\033[0m' COLOR_INV="\033[7m"
COLOR_RED='\033[31m' COLOR_GREEN='\033[32m' COLOR_BLUE='\033[34m' COLOR_GREY='\033[90m'
log_error() { echo -e "${COLOR_INV}${COLOR_RED}[!]${COLOR_RST} $*" >&2; }
log_stage() { echo -e "${COLOR_INV}${COLOR_GREEN}[*]${COLOR_RST} $*" >&2; }
log_action() { echo -e "${COLOR_INV}${COLOR_BLUE}[-]${COLOR_RST} $*" >&2; }
log_verbose() { echo -e "${COLOR_INV}${COLOR_GREY}[.]${COLOR_RST}${COLOR_GREY} $*${COLOR_RST}" >&2; }

PROJECT_ROOT="$(realpath "$(dirname "$0")"/..)"

init_shell_options() { set -eu -o pipefail; }

chdir_root() { cd "$PROJECT_ROOT"; }

# Shorthand for running the standard uv.
_uv() { uv --directory="$PROJECT_ROOT/compiler" "$@"; }

init_py_env() {
    init_shell_options
    chdir_root
    cd compiler
    source .venv/bin/activate
    set -x
}
