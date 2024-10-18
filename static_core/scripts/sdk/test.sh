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
set -x

PANDA_SDK_BUILD_TYPE="${1:-"Release"}"

SCRIPT_DIR="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

BUILD_DIR=${BUILD_DIR:-'.'}
mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR"

OHOS_SDK_NATIVE_URL=${OHOS_SDK_NATIVE_URL:-'https://contentcenter-drcn.dbankcdn.com/pub_1/DevEcoSpace_1_900_9/4a/v3/oidhh45PQ-qFyqu2pO5Kmw/nZgBfyRtTzuaBczj9WgsWQ.zip'}

# Search for default OHOS SDK native or download it
if [ -z "$OHOS_SDK_NATIVE" ]; then
    OHOS_SDK_NATIVE_DEFAULT_1=/opt/ohos-sdk/native
    OHOS_SDK_NATIVE_DEFAULT_2="$(realpath ./native)"
    if [ -d "$OHOS_SDK_NATIVE_DEFAULT_1" ]; then
        OHOS_SDK_NATIVE="$OHOS_SDK_NATIVE_DEFAULT_1"
    elif [ -d "$OHOS_SDK_NATIVE_DEFAULT_2" ]; then
        OHOS_SDK_NATIVE="$OHOS_SDK_NATIVE_DEFAULT_2"
    else
        if [ -z "$OHOS_SDK_NATIVE_URL" ]; then
            echo "Error: OHOS SDK not found, please set OHOS_SDK_NATIVE or OHOS_SDK_NATIVE_URL environment variable"
            exit 1
        fi
        curl --retry 5 -Lo ohos-sdk-native.zip "${OHOS_SDK_NATIVE_URL}"
        unzip -q ohos-sdk-native.zip
        OHOS_SDK_NATIVE=native
    fi
fi

# Build panda SDK
SDK_VERSION=$(grep '"version":' "$SCRIPT_DIR"/package.json | sed 's|.*"version":.*"\(.*\)".*|\1|')
"$SCRIPT_DIR"/build_sdk.sh "$OHOS_SDK_NATIVE" build-sdk "$PANDA_SDK_BUILD_TYPE"

ACTUAL="/tmp/panda_sdk_test_actual.txt"
EXPECTED="/tmp/panda_sdk_test_expected.txt"

# Test 1
find build-sdk -maxdepth 1 | sort --version-sort >"$ACTUAL"
echo "build-sdk
build-sdk/linux_arm64_host_tools
build-sdk/linux_host_tools
build-sdk/ohos_arm64
build-sdk/panda-sdk-$SDK_VERSION.tgz
build-sdk/sdk
build-sdk/windows_host_tools" >"$EXPECTED"
diff "$ACTUAL" "$EXPECTED"

# Test 2
find build-sdk/sdk/ -maxdepth 3 | sort --version-sort >"$ACTUAL"
EXPECTED_TEST2="$SCRIPT_DIR/test2_file_list_expected.txt"
diff "$ACTUAL" "$EXPECTED_TEST2"

rm "$ACTUAL" "$EXPECTED"
