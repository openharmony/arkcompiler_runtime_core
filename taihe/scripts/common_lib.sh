#!/bin/bash
# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
TAIHE_FFI_GEN_VERSION="1.9.1"
TAIHE_FFI_GEN_REPO_URLS=(
    # https
    "https://gitcode.com/openharmony-sig/arkcompiler_taihe_ffi_gen.git"
    # SSH
    # "git@gitcode.com:openharmony-sig/arkcompiler_taihe_ffi_gen.git"
)
TAIHE_FFI_GEN_RESTORED_PATHS=(
    compiler
    runtime
    test
    cmake
)
TAIHE_LOCAL_RESOURCES_REL_PATH="compiler/taihe/utils/resources.py"

init_shell_options() { set -eu -o pipefail; }

chdir_root() { cd "$PROJECT_ROOT"; }

get_taihe_ffi_gen_branch() {
    echo "${TAIHE_FFI_GEN_BRANCH:-v$TAIHE_FFI_GEN_VERSION}"
}

get_local_taihe_resources_source() {
    if [[ -n "${TAIHE_LOCAL_RESOURCES_PATH:-}" ]]; then
        echo "$TAIHE_LOCAL_RESOURCES_PATH"
        return 0
    fi
    echo "$PROJECT_ROOT/$TAIHE_LOCAL_RESOURCES_REL_PATH"
}

backup_local_taihe_resources() {
    local backup_root="$1"
    local source_path
    local project_source_path="$PROJECT_ROOT/$TAIHE_LOCAL_RESOURCES_REL_PATH"
    source_path="$(get_local_taihe_resources_source)"

    if [[ ! -f "$source_path" ]]; then
        return 0
    fi
    if [[ -e "$source_path" && ! -f "$source_path" ]]; then
        log_error "Local Taihe source path must be a file: $source_path"
        return 1
    fi

    mkdir -p "$backup_root/$(dirname "$TAIHE_LOCAL_RESOURCES_REL_PATH")"
    if [[ "$source_path" == "$project_source_path" ]]; then
        log_stage "Temporarily move local $TAIHE_LOCAL_RESOURCES_REL_PATH"
        mv "$source_path" "$backup_root/$TAIHE_LOCAL_RESOURCES_REL_PATH"
    else
        log_stage "Save local $TAIHE_LOCAL_RESOURCES_REL_PATH from $source_path"
        cp -a "$source_path" "$backup_root/$TAIHE_LOCAL_RESOURCES_REL_PATH"
    fi
}

restore_local_taihe_resources() {
    local backup_root="$1"
    local backup_path="$backup_root/$TAIHE_LOCAL_RESOURCES_REL_PATH"
    local restore_path="$PROJECT_ROOT/$TAIHE_LOCAL_RESOURCES_REL_PATH"

    if [[ ! -f "$backup_path" ]]; then
        return 0
    fi

    mkdir -p "$PROJECT_ROOT/$(dirname "$TAIHE_LOCAL_RESOURCES_REL_PATH")"
    cp -a "$backup_path" "$restore_path"
    log_stage "Restore local $TAIHE_LOCAL_RESOURCES_REL_PATH"
}

restore_missing_taihe_sources() {
    local has_missing_paths=0
    local backup_root
    local repo_urls=("${TAIHE_FFI_GEN_REPO_URLS[@]}")
    local rel_path
    for rel_path in "${TAIHE_FFI_GEN_RESTORED_PATHS[@]}"; do
        if [[ ! -e "$PROJECT_ROOT/$rel_path" ]]; then
            has_missing_paths=1
            break
        fi
    done

    if [[ $has_missing_paths -eq 0 ]]; then
        return 0
    fi

    if ! command -v git &>/dev/null; then
        log_error "git is required to restore missing Taihe sources"
        return 1
    fi

    mkdir -p "$PROJECT_ROOT/.cache"

    local branch clone_root repo_url
    local cloned=0
    branch="$(get_taihe_ffi_gen_branch)"
    if [[ -n "${TAIHE_FFI_GEN_REPO:-}" ]]; then
        repo_urls=("$TAIHE_FFI_GEN_REPO" "${repo_urls[@]}")
    fi
    clone_root="$(mktemp -d "$PROJECT_ROOT/.cache/arkcompiler_taihe_ffi_gen.XXXXXX")"
    backup_root="$clone_root/local_backup"
    backup_local_taihe_resources "$backup_root"

    for repo_url in "${repo_urls[@]}"; do
        rm -rf "$clone_root/repo"
        log_stage "Fetch arkcompiler_taihe_ffi_gen branch $branch from $repo_url"
        if git clone --depth 1 --branch "$branch" "$repo_url" "$clone_root/repo"; then
            cloned=1
            break
        fi
        log_error "Failed to fetch $repo_url branch $branch"
    done

    if [[ $cloned -ne 1 ]]; then
        restore_local_taihe_resources "$backup_root"
        rm -rf "$clone_root"
        log_error "Unable to fetch arkcompiler_taihe_ffi_gen branch $branch"
        return 1
    fi

    local src_path
    for rel_path in "${TAIHE_FFI_GEN_RESTORED_PATHS[@]}"; do
        src_path="$clone_root/repo/$rel_path"
        if [[ ! -e "$src_path" ]]; then
            restore_local_taihe_resources "$backup_root"
            rm -rf "$clone_root"
            log_error "arkcompiler_taihe_ffi_gen is missing $rel_path"
            return 1
        fi
        log_stage "Restore $rel_path from arkcompiler_taihe_ffi_gen"
        rm -rf "$PROJECT_ROOT/$rel_path"
        cp -a "$src_path" "$PROJECT_ROOT/"
    done

    restore_local_taihe_resources "$backup_root"
    rm -rf "$clone_root"
}

# Shorthand for running the standard uv.
_uv() { uv --directory="$PROJECT_ROOT" "$@"; }

init_py_env() {
    init_shell_options
    chdir_root
    source .venv/bin/activate
    export PYTHONPATH="$PROJECT_ROOT/compiler"
    set -x
}
