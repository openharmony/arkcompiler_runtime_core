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

readonly TEMPLATES_DIR="${WD}/templates"
readonly TESTS_SRC_TEMPLATE="${TEMPLATES_DIR}/test.template.ts"
readonly TESTS_MODEL_TEMPLATE="${TEMPLATES_DIR}/model.template.d.ts"

readonly OUT_DIR="${WD}/out"
readonly TESTS_SRC_DIR="${OUT_DIR}/src"
readonly TESTS_MODEL_DIR="${OUT_DIR}/model"
readonly TESTS_OUT_DIR="${OUT_DIR}/out"

readonly EXT_TS="ts"
readonly EXT_DECL_ETS="d.sts"
readonly EXT_DECL_TS="d.ts"

function panic() {
    local msg="${1-}"
    echo "${msg}"
    exit 1
}

function assert_path() {
    local path="${1?}"
    if [ ! -f "${path}" ] && [ ! -d "${path}" ]; then
        panic "no path ${path} in the filesystem"
    fi
}

function clear_dir() {
    local dir="${1?}"
    rm -r -f "${dir}"
    mkdir -p "${dir}"
}

function gen_tests() {
    local jsvalue="JSValue"

    declare -A TS2ETS=(
        # ["AsyncFunction"]="${jsvalue}"
        # ["Reflect"]="${jsvalue}"
        # ["AsyncIterator"]="${jsvalue}"
        # ["Proxy"]="${jsvalue}"
        # ["TypedArray"]="${jsvalue}"
        # ["Array"]="Array"
        # ["Iterator"]="${jsvalue}"
        # ["Map"]="Map"
        # ["Set"]="Set"
        # ["WeakMap"]="WeakMap"
        # ["WeakSet"]="WeakSet"

        # unsupported types - translated to the JSValue
        ["Object"]="${jsvalue}"
        ["Symbol"]="${jsvalue}"
        ["Function"]="${jsvalue}"
        ["AsyncGenerator"]="${jsvalue}"
        ["AsyncGeneratorFunction"]="${jsvalue}"
        ["Generator"]="${jsvalue}"
        ["GeneratorFunction"]="${jsvalue}"
        ["unknown"]="${jsvalue}"
        ["any"]="${jsvalue}"
        ["undefined"]="${jsvalue}"

        # direct translation
        ["null"]="null"
        ["String"]="String"
        ["string"]="string"
        ["bigint"]="bigint"
        ["BigInt"]="BigInt"
        ["Boolean"]="Boolean"
        ["Number"]="Number"
        ["number"]="number"
        ["BigInt64Array"]="BigInt64Array"
        ["BigUint64Array"]="BigUint64Array"
        ["DataView"]="DataView"
        ["Float32Array"]="Float32Array"
        ["Float64Array"]="Float64Array"
        ["ArrayBuffer"]="ArrayBuffer"
        ["Date"]="Date"
        ["Error"]="Error"
        ["EvalError"]="Error"
        ["FinalizationRegistry"]="FinalizationRegistry"
        ["RangeError"]="RangeError"
        ["ReferenceError"]="ReferenceError"
        ["RegExp"]="RegExp"
        ["SharedArrayBuffer"]="SharedArrayBuffer"
        ["URIError"]="URIError"
        ["SyntaxError"]="SyntaxError"
        ["TypeError"]="TypeError"
    )
    readonly TS2ETS

    assert_path "${TESTS_SRC_TEMPLATE}"
    assert_path "${TESTS_MODEL_TEMPLATE}"

    clear_dir "${TESTS_SRC_DIR}"
    clear_dir "${TESTS_MODEL_DIR}"

    for TS_TYPE in "${!TS2ETS[@]}"; do
        ETS_TYPE="${TS2ETS[${TS_TYPE}]}"

        echo "generating ${TS_TYPE} -> ${ETS_TYPE} test..."

        sed "s@TYPE@${TS_TYPE}@g" "${TESTS_SRC_TEMPLATE}" >"${TESTS_SRC_DIR}/${TS_TYPE}2${ETS_TYPE}.${EXT_TS}"
        sed "s@TYPE@${ETS_TYPE}@g" "${TESTS_MODEL_TEMPLATE}" >"${TESTS_MODEL_DIR}/${TS_TYPE}2${ETS_TYPE}.${EXT_DECL_ETS}"
    done

}

function build_tool() {
    cd "${WD}"

    npm i
    npm run compile
}

function run_tests() {
    assert_path "${TESTS_SRC_DIR}"

    for TEST_FILE in "${TESTS_SRC_DIR}"/*"${EXT_TS}"; do
        TEST_NAME="$(basename -- "${TEST_FILE}")"
        TEST_NAME="${TEST_NAME%.*}"

        MODEL_FILE="${TESTS_MODEL_DIR}/${TEST_NAME}.${EXT_DECL_ETS}"
        assert_path "${MODEL_FILE}"

        echo "running ${TEST_NAME} test"
        echo "command:"
        echo "FNAME=${TEST_FILE} OUT_PATH=${TESTS_OUT_DIR} npm run declgen"

        if ! FNAME="${TEST_FILE}" OUT_PATH="${TESTS_OUT_DIR}" npm run declgen >/dev/null; then
            panic "declgen failed! see output for details"
        fi

        OUT_FILE="${TESTS_OUT_DIR}/${TEST_NAME}.${EXT_DECL_TS}"
        assert_path "${OUT_FILE}"

        if ! diff "${OUT_FILE}" "${MODEL_FILE}"; then
            panic "diff produced output. tests failed"
        fi
    done
}

function main() {
    build_tool
    gen_tests
    run_tests

    echo "done."
}

main
