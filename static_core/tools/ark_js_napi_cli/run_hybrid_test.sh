#!/bin/bash
# Copyright (c) 2024 Huawei Device Co., Ltd.
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

set -e

declare BUILD_ROOT=""
declare JS_TEST_PATH=""
declare STS_TEST_PATH=""
declare TS_TEST_PATH=""
declare TS_COMMON_PATH=""

while [[ -n "$1" ]]; do
    case "$1" in
        -b|--build-dir)
            shift
            BUILD_ROOT="$1"
            ;;
        --js-test)
            shift
            JS_TEST_PATH="$1"
            ;;
        --sts-test)
            shift
            STS_TEST_PATH="$1"
            ;;
        --ts-test)
            shift
            TS_TEST_PATH="$1"
            ;;
        --ts-common)
            shift
            TS_COMMON_PATH="$1"
            ;;
        *)
            echo "Unknow option '$1'"
            ;;
    esac
    shift
done

if [[ -z "${BUILD_ROOT}"  ]]; then
    echo "Directory with build was not set. Please use -b/--build-dir option"
    exit 1
fi
if [[ ! -d "${BUILD_ROOT}" ]]; then
    echo "Build directory does not exist: ${BUILD_ROOT}"
    exit 1
fi
if [[ -z "${JS_TEST_PATH}" && -z "${TS_TEST_PATH}" ]]; then
    echo "JS or TS test file was not set. Please use --js-test or --ts-test option"
    exit 1
fi
if [[ ! -f "${JS_TEST_PATH}" && ! -f "${TS_TEST_PATH}" ]]; then
    echo "JS or TS test file does not exist: ${JS_TEST_PATH} or ${TS_TEST_PATH}"
    exit 1
fi
if [[ -n "${STS_TEST_PATH}" && ! -f "${STS_TEST_PATH}" ]]; then
    echo "STS test file does not exist: ${STS_TEST_PATH}"
    exit 1
fi

## Prepare test space
declare TEST_SPACE="${BUILD_ROOT}/hybrid_test"
declare JS_TEST_NAME="" STS_TEST_NAME="" TS_TEST_NAME="" TS_COMMON_NAME=""
JS_TEST_NAME="$(basename -s .js "${JS_TEST_PATH}")_ts"
TS_TEST_NAME="$(basename -s .ts "${TS_TEST_PATH}")_ts"
if [[ -n "${STS_TEST_PATH}" ]]; then
    STS_TEST_NAME="$(basename -s .sts "${STS_TEST_PATH}")_sts"
fi

if [[ -n "${TS_COMMON_PATH}" ]]; then
    TS_COMMON_NAME="$(basename -s .ts "${TS_COMMON_PATH}")_ts"
fi

if [[ -d "${TEST_SPACE}" ]]; then
    rm -rf "${TEST_SPACE}"
    echo "${TEST_SPACE} has removed"
fi

set -x

mkdir -p "${TEST_SPACE}/bin"
mkdir -p "${TEST_SPACE}/lib"
mkdir -p "${TEST_SPACE}/module"

### Copy binaries
cp "${BUILD_ROOT}/arkcompiler/ets_frontend/es2abc" "${TEST_SPACE}/bin"
cp "${BUILD_ROOT}/arkcompiler/ets_frontend/es2panda" "${TEST_SPACE}/bin"
cp "${BUILD_ROOT}/arkcompiler/runtime_core/ark_js_napi_cli" "${TEST_SPACE}/bin"

### Copy libraries
cp "${BUILD_ROOT}/arkcompiler/runtime_core"/*.so "${TEST_SPACE}/lib"
cp "${BUILD_ROOT}/arkcompiler/ets_runtime"/*.so "${TEST_SPACE}/lib"
cp "${BUILD_ROOT}/gen/arkcompiler/ets_runtime/stub.an" "${TEST_SPACE}/lib"
cp "${BUILD_ROOT}/arkui/napi"/*.so "${TEST_SPACE}/lib"
cp "${BUILD_ROOT}/thirdparty/icu"/*.so "${TEST_SPACE}/lib"
cp "${BUILD_ROOT}/thirdparty/libuv"/*.so "${TEST_SPACE}/lib"
cp "${BUILD_ROOT}/thirdparty/bounds_checking_function"/*.so "${TEST_SPACE}/lib"

### Copy STS specific files
cp "${BUILD_ROOT}/arkcompiler/runtime_core/libets_interop_js_napi.so" "${TEST_SPACE}/module"
cp "${BUILD_ROOT}/gen/arkcompiler/runtime_core/static_core/plugins/ets/etsstdlib.abc" "${TEST_SPACE}"
cp "${BUILD_ROOT}/arkcompiler/ets_frontend/arktsconfig.json" "${TEST_SPACE}"

## Test process 
cd "${TEST_SPACE}"

### Compile test
if [[ -n "${TS_TEST_PATH}" ]]; then
    "${TEST_SPACE}"/bin/es2abc --merge-abc --extension=ts --module "${TS_TEST_PATH}" --output "${TEST_SPACE}/${TS_TEST_NAME}.abc"
else
    "${TEST_SPACE}"/bin/es2abc --merge-abc --extension=js --commonjs "${JS_TEST_PATH}" --output "${TEST_SPACE}/${JS_TEST_NAME}.abc"
fi
if [[ -n "${TS_COMMON_PATH}" ]]; then
    "${TEST_SPACE}"/bin/es2abc --merge-abc --extension=ts --module "${TS_COMMON_PATH}" --output "${TEST_SPACE}/${TS_COMMON_NAME}.abc"
fi
if [[ -n "${STS_TEST_PATH}" ]]; then
    "${TEST_SPACE}"/bin/es2panda --extension=sts --opt-level=2 \
                                 --arktsconfig="${TEST_SPACE}/arktsconfig.json" \
                                 --output="${TEST_SPACE}/${STS_TEST_NAME}.abc" \
                                 "${STS_TEST_PATH}"
fi

### Run test
if [[ -n "${TS_TEST_PATH}" ]]; then
    LD_LIBRARY_PATH="${TEST_SPACE}/lib" "${TEST_SPACE}"/bin/ark_js_napi_cli \
        --stub-file "${TEST_SPACE}/lib/stub.an" \
        --entry-point "${TS_TEST_NAME}" \
        "${TEST_SPACE}/${TS_TEST_NAME}.abc"
else
    LD_LIBRARY_PATH="${TEST_SPACE}/lib" "${TEST_SPACE}"/bin/ark_js_napi_cli \
            --stub-file "${TEST_SPACE}/lib/stub.an" \
            --entry-point "${JS_TEST_NAME}" \
            "${TEST_SPACE}/${JS_TEST_NAME}.abc"
fi
exit $?
