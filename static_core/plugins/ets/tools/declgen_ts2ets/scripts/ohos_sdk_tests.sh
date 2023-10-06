#!/bin/bash
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

panic() {
    local -r MSG="${1-}"
    echo "${MSG}"
    exit 1
}

assert_path() {
    local -r PATH="${1?}"
    if [ ! -f "${PATH}" ] && [ ! -d "${PATH}" ]; then
        panic "no path ${PATH} in the filesystem"
    fi
}

clear_dir() {
    local -r DIR="${1?}"
    rm -rf "${DIR}"
    mkdir -p "${DIR}"
}

build_tool() {
    cd "${WD}"
    npm i
    npm run compile
}

declgen_ohos_api() {
    local -r OHOS_SDK_API="$(realpath "${1?}")"
    local -r OUT_BASE="${2?}"
    local -r TMP_OHOS_SDK_PATH="/tmp/ohos_sdk"

    assert_path "${OHOS_SDK_API}"
    clear_dir "${TMP_OHOS_SDK_PATH}"
    clear_dir "${OUT_BASE}"

    local -r COUNT="$(find "${OHOS_SDK_API}" -name "*.${EXT_DECL_TS}" | wc -l)"

    local FNAME
    local DIRNAME
    local TMP_FILE
    local OUT
    local I=-1
    while read -r FILE; do
        I=$((I + 1))
        FNAME="$(basename "${FILE}")"
        FNAME="${FNAME%."${EXT_DECL_TS}"}"
        DIRNAME="$(dirname "${FILE}")"
        DIRNAME="${DIRNAME##"${OHOS_SDK_API}"}"

        OUT="${OUT_BASE}/${DIRNAME}"
        mkdir -p "${OUT}"

        TMP_FILE="${TMP_OHOS_SDK_PATH}/${FNAME}.${EXT_TS}"
        cp "${FILE}" "${TMP_FILE}"

        echo "[${I}/${COUNT}] processing ${FILE}"
        if ! FNAME="${TMP_FILE}" OUT_PATH="${OUT}" npm run declgen >/dev/null; then
            panic "declgen failed for file ${FILE}!"
        fi
    done < <(find "${OHOS_SDK_API}" -name "*.${EXT_DECL_TS}")
}

gen_arktsconfig() {
    local -r ARKTSCONFIG="${1?}"
    local -r BASE_URL="${2?}"

    assert_path "${BASE_URL}"

    cat >"${ARKTSCONFIG}" <<EOL
{
  "compilerOptions": {
    "baseUrl": "${BASE_URL}",
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

run_es2panda() {
    local -r ARKTSCONFIG="${1?}"
    local -r INPUT="${2?}"
    local -r OUTPUT="${3?}"

    assert_path "${ES2PANDA}"
    assert_path "${ARKTSCONFIG}"
    assert_path "${INPUT}"

    echo "running es2panda on ${INPUT}..."
    if ! ${ES2PANDA} --arktsconfig="${ARKTSCONFIG}" --output "${OUTPUT}" "${INPUT}"; then
        panic "es2panda failed!"
    fi
}

try_get_sdk_from_url() {
    local -r URL="${1?}"
    local -r OHOS_SDK_DST="${2?}"

    if ! curl --retry 5 -Lo "${OHOS_SDK_DST}.zip" "${URL}"; then
        panic "OHOS_SDK_PATH is neither a valid path nor a valid URL!"
    fi

    if ! unzip "${OHOS_SDK_DST}.zip" -d "${OHOS_SDK_DST}"; then
        panic "can't unzip ${OHOS_SDK_DST}.zip!"
    fi
}

main() {
    local -r ARKTSCONFIG_PATH="${OUT_DIR}/arktsconfig.json"
    local -r MAIN_TS="${WD}/tests/ohos_sdk/main.ets"
    local -r MAIN_ABC="${OUT_DIR}/main.abc"

    build_tool

    declgen_ohos_api "${OHOS_SDK_PATH}/${OHOS_SDK_JS_API_REL_PATH}" "${OHOS_SDK_API_DECL_PATH}"
    gen_arktsconfig "${ARKTSCONFIG_PATH}" "${OHOS_SDK_API_DECL_PATH}"
    run_es2panda "${ARKTSCONFIG_PATH}" "${MAIN_TS}" "${MAIN_ABC}"

    echo "done."
}

main
