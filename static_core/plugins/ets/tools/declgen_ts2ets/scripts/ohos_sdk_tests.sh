#!/bin/bash
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

set -eu -o pipefail

WD="$(realpath "$(dirname "${0}")/..")"
readonly WD

readonly OHOS_SDK_PATH="${1?}"
readonly OHOS_SDK_JS_API_REL_PATH="js/api/"

readonly PANDA_BUILD_PATH="${2?}"
readonly ES2PANDA="${PANDA_BUILD_PATH}/bin/es2panda"

readonly OUT_DIR="${WD}/out"
readonly OHOS_SDK_API_DECL_PATH="${OUT_DIR}/decl"

readonly EXT_TS="ts"
readonly EXT_DECL_TS="d.ts"

function panic() {
    local -r msg="${1-}"
    echo "${msg}"
    exit 1
}

function assert_path() {
    local -r path="${1?}"
    if [ ! -f "${path}" ] && [ ! -d "${path}" ]; then
        panic "no path ${path} in the filesystem"
    fi
}

function clear_dir() {
    local -r dir="${1?}"
    rm -r -f "${dir}"
    mkdir -p "${dir}"
}

function build_tool() {
    cd "${WD}"
    npm i
    npm run compile
}

function declgen_ohos_api() {
    local -r ohos_sdk_api="$(realpath "${1?}")"
    local -r out_base="${2?}"
    local -r tmp_ohos_sdk_path="/tmp/ohos_sdk"

    assert_path "${ohos_sdk_api}"
    clear_dir "${tmp_ohos_sdk_path}"
    clear_dir "${out_base}"

    local -r count="$(find "${ohos_sdk_api}" -name "*.${EXT_DECL_TS}" | wc -l)"

    local fname
    local dir_name
    local tmp_file
    local out
    local i=-1
    while read -r FILE; do
        i=$((i + 1))
        fname="$(basename "${FILE}")"
        fname="${fname%."${EXT_DECL_TS}"}"
        dir_name="$(dirname "${FILE}")"
        dir_name="${dir_name##"${ohos_sdk_api}"}"

        out="${out_base}/${dir_name}"
        mkdir -p "${out}"

        tmp_file="${tmp_ohos_sdk_path}/${fname}.${EXT_TS}"
        cp "${FILE}" "${tmp_file}"

        echo "[${i}/${count}] processing ${FILE}"
        if ! fname="${tmp_file}" OUT_PATH="${out}" npm run declgen >/dev/null; then
            panic "declgen failed for file ${FILE}!"
        fi
    done < <(find "${ohos_sdk_api}" -name "*.${EXT_DECL_TS}")
}

function gen_arktsconfig() {
    local -r arktsconfig="${1?}"
    local -r base_url="${2?}"

    assert_path "${base_url}"

    cat >"${arktsconfig}" <<EOL
{
  "compilerOptions": {
    "baseUrl": "${base_url}",
    "paths": {
        "std": ["${WD}/../../stdlib/std"],
        "escompat": ["${WD}/../../stdlib/escompat"]
    },
    "dynamicPaths": {
    }
  }
}
EOL
}

function run_es2panda() {
    local -r arktsconfig="${1?}"
    local -r input="${2?}"
    local -r output="${3?}"

    assert_path "${ES2PANDA}"
    assert_path "${arktsconfig}"
    assert_path "${input}"

    echo "running es2panda on ${input}..."
    if ! ${ES2PANDA} --arktsconfig="${arktsconfig}" --output "${output}" "${input}"; then
        panic "es2panda failed!"
    fi
}

function try_get_sdk_from_url() {
    local -r url="${1?}"
    local -r ohos_sdk_dst="${2?}"

    if ! curl --retry 5 -Lo "${ohos_sdk_dst}.zip" "${url}"; then
        panic "OHOS_SDK_PATH is neither a valid path nor a valid url!"
    fi

    if ! unzip "${ohos_sdk_dst}.zip" -d "${ohos_sdk_dst}"; then
        panic "can't unzip ${ohos_sdk_dst}.zip!"
    fi
}

function main() {
    local -r arktsconfig_path="${OUT_DIR}/arktsconfig.json"
    local -r main_ts="${WD}/tests/ohos_sdk/main.ets"
    local -r main_abc="${OUT_DIR}/main.abc"

    build_tool

    declgen_ohos_api "${OHOS_SDK_PATH}/${OHOS_SDK_JS_API_REL_PATH}" "${OHOS_SDK_API_DECL_PATH}"
    gen_arktsconfig "${arktsconfig_path}" "${OHOS_SDK_API_DECL_PATH}"
    run_es2panda "${arktsconfig_path}" "${main_ts}" "${main_abc}"

    echo "done."
}

main
