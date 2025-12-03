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

set -o errexit -o nounset -o pipefail

declare exec_file="" output_file=""
declare -a env_variables=()
declare ld_library_path=""
declare -a prlimit_cmd=()

while [[ -n "${1}" ]]; do
    case "${1}" in
        --exec-file)
            exec_file="${2}"
            shift
            ;;
        --output-file)
            output_file="${2}"
            shift
            ;;
        --env)
            env_variables+=("${2}")
            shift
            ;;
        --libs-path)
            ld_library_path="${2}"
            shift
            ;;
        --no-cores)
            prlimit_cmd=("prlimit" "--core=0")
            ;;
        *)
            break
            ;;
    esac
    shift
done

if [[ -z "${exec_file}" ]]; then
    echo "Executable file is not set"
    exit 1
fi
if [[ -z "${output_file}" ]]; then
    echo "Output file is not set"
    exit 1
fi

env "LD_LIBRARY_PATH=${ld_library_path}" "${env_variables[@]}" \
"${prlimit_cmd[@]}" timeout --preserve-status --signal=USR1 --kill-after=30s 20m \
"${exec_file}" "${@}" \
    > "${output_file}" 2>&1 \
    || (cat "${output_file}" && false)
