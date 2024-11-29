#!/usr/bin/env bash
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

set -eo pipefail

export SCRIPT_DIR="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"
export ARK_ROOT="$SCRIPT_DIR/../.."

export BUILD_TYPE_RELEASE="Release"
BUILD_TYPE_FAST_VERIFY="FastVerify"
BUILD_TYPE_DEBUG="Debug"

export OHOS_SDK_NATIVE="$(realpath "$1")"
export SDK_BUILD_ROOT="$(realpath "$2")"
export PANDA_SDK_BUILD_TYPE="${3:-"$BUILD_TYPE_RELEASE"}"
export PANDA_LLVM_BACKEND="${4:-"OFF"}"
export PANDA_BUILD_LLVM_BINARIES="${5:-"OFF"}"

function usage() {
    echo "$(basename "${BASH_SOURCE[0]}") path/to/ohos/sdk/native path/to/panda/sdk/destination build_type:[$BUILD_TYPE_RELEASE,$BUILD_TYPE_FAST_VERIFY,$BUILD_TYPE_DEBUG]"
    exit 1
}

case "$PANDA_SDK_BUILD_TYPE" in
"$BUILD_TYPE_RELEASE" | "$BUILD_TYPE_FAST_VERIFY" | "$BUILD_TYPE_DEBUG") ;;
*)
    echo "Invalid build_type option!"
    usage
    ;;
esac

if [ ! -d "$OHOS_SDK_NATIVE" ]; then
    echo "Error: No such directory: $OHOS_SDK_NATIVE"
    usage
fi

if [ -z "$SDK_BUILD_ROOT" ]; then
    echo "Error: path to panda sdk destination is not provided"
    usage
fi

export PANDA_SDK_PATH="$SDK_BUILD_ROOT/sdk"

rm -r -f "$PANDA_SDK_PATH"

source "$SCRIPT_DIR"/build_sdk_lib
ohos
linux_arm64_tools
linux_tools
windows_tools
ts_linter
ets_std_lib

echo "> Packing NPM package..."
cp "$SCRIPT_DIR"/package.json "$PANDA_SDK_PATH"
cd "$PANDA_SDK_PATH"
npm pack --pack-destination "$SDK_BUILD_ROOT"
