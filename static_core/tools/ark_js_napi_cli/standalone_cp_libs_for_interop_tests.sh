#!/bin/bash
# Copyright (c) 2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e

MODE=$1

if [[ -z "${PANDA_BUILD_PATH}" ]]; then
    echo "Error: 'PANDA_BUILD_PATH' env variable must be non-empty"
    exit 1
fi

if [[ -z "${ARK_STANDALONE_BUILD_ROOT}" ]]; then
    echo "Error: 'ARK_STANDALONE_BUILD_ROOT' env variable must be non-empty"
    exit 1
fi

if [[ "${MODE}" != "release" && "${MODE}" != "debug" ]]; then
    echo "Error: 'MODE' parameter must be passed with one of the following values: 'release' or 'debug'"
    echo "Current 'MODE' value is \"$MODE\""
    exit 1
fi

PANDA_LIBRARIES_PATH="$PANDA_BUILD_PATH/lib/arkjsvm_interop/"
PANDA_BINARIES_PATH="$PANDA_BUILD_PATH/bin/arkjsvm_interop/"

mkdir -p "$PANDA_BUILD_PATH"
mkdir -p "$PANDA_LIBRARIES_PATH"
mkdir -p "$PANDA_BINARIES_PATH"

REQUIRED_LIBS=(
    "$ARK_STANDALONE_BUILD_ROOT/out/x64.$MODE/arkui/napi/libace_napi.so"
    "$ARK_STANDALONE_BUILD_ROOT/out/x64.$MODE/arkcompiler/ets_runtime/libark_jsruntime.so"
    "$ARK_STANDALONE_BUILD_ROOT/out/x64.$MODE/thirdparty/libuv/libuv.so"
    "$ARK_STANDALONE_BUILD_ROOT/out/x64.$MODE/thirdparty/icu/libhmicui18n.so"
    "$ARK_STANDALONE_BUILD_ROOT/out/x64.$MODE/thirdparty/bounds_checking_function/libsec_shared.so"
    "$ARK_STANDALONE_BUILD_ROOT/out/x64.$MODE/thirdparty/icu/libhmicuuc.so"
    "$ARK_STANDALONE_BUILD_ROOT/out/x64.$MODE/arkcompiler/runtime_core/libinterop_test_helper.so"
)

for lib in "${REQUIRED_LIBS[@]}"; do
    cp "$lib" "$PANDA_LIBRARIES_PATH"
done

REQUIRED_BINARIES=(
    "$ARK_STANDALONE_BUILD_ROOT/out/x64.$MODE/arkcompiler/runtime_core/ark_js_napi_cli"
    "$ARK_STANDALONE_BUILD_ROOT/out/x64.$MODE/arkcompiler/ets_frontend/es2abc"
)

for binary in "${REQUIRED_BINARIES[@]}"; do
    cp "$binary" "$PANDA_BINARIES_PATH"
done

exit 0
