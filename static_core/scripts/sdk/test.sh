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
build-sdk/linux_host_tools
build-sdk/ohos_arm64
build-sdk/panda-sdk-$SDK_VERSION.tgz
build-sdk/sdk
build-sdk/windows_host_tools" >"$EXPECTED"
diff "$ACTUAL" "$EXPECTED"

# Test 2
find build-sdk/sdk/ -maxdepth 3 | sort --version-sort >"$ACTUAL"
echo 'build-sdk/sdk/
build-sdk/sdk/ets
build-sdk/sdk/ets/etsstdlib.abc
build-sdk/sdk/ets/stdlib
build-sdk/sdk/ets/stdlib/README.md
build-sdk/sdk/ets/stdlib/escompat
build-sdk/sdk/ets/stdlib/native
build-sdk/sdk/ets/stdlib/std
build-sdk/sdk/linux_host_tools
build-sdk/sdk/linux_host_tools/bin
build-sdk/sdk/linux_host_tools/bin/ark
build-sdk/sdk/linux_host_tools/bin/ark_aot
build-sdk/sdk/linux_host_tools/bin/ark_disasm
build-sdk/sdk/linux_host_tools/bin/ark_link
build-sdk/sdk/linux_host_tools/bin/es2panda
build-sdk/sdk/linux_host_tools/lib
build-sdk/sdk/linux_host_tools/lib/libarkaotmanager.so
build-sdk/sdk/linux_host_tools/lib/libarkassembler.so
build-sdk/sdk/linux_host_tools/lib/libarkbase.so
build-sdk/sdk/linux_host_tools/lib/libarkbytecodeopt.so
build-sdk/sdk/linux_host_tools/lib/libarkcompiler.so
build-sdk/sdk/linux_host_tools/lib/libarkdisassembler.so
build-sdk/sdk/linux_host_tools/lib/libarkfile.so
build-sdk/sdk/linux_host_tools/lib/libarklinker.so
build-sdk/sdk/linux_host_tools/lib/libarkruntime.so
build-sdk/sdk/linux_host_tools/lib/libarkziparchive.so
build-sdk/sdk/linux_host_tools/lib/libc_secshared.so
build-sdk/sdk/linux_host_tools/lib/libe2p_test_plugin.so
build-sdk/sdk/linux_host_tools/lib/libes2panda-lib.so
build-sdk/sdk/linux_host_tools/lib/libes2panda-public.so
build-sdk/sdk/linux_host_tools/lib/libetsnative.so
build-sdk/sdk/linux_host_tools/lib/libhmicui18n.z.so
build-sdk/sdk/linux_host_tools/lib/libhmicuuc.z.so
build-sdk/sdk/linux_host_tools/lib/libinit_icu.so
build-sdk/sdk/linux_host_tools/lib/libirtoc.so
build-sdk/sdk/ohos_arm64
build-sdk/sdk/ohos_arm64/bin
build-sdk/sdk/ohos_arm64/bin/ark
build-sdk/sdk/ohos_arm64/bin/ark_aot
build-sdk/sdk/ohos_arm64/include
build-sdk/sdk/ohos_arm64/include/compiler
build-sdk/sdk/ohos_arm64/include/libpandabase
build-sdk/sdk/ohos_arm64/include/libpandafile
build-sdk/sdk/ohos_arm64/include/panda_gen_options
build-sdk/sdk/ohos_arm64/include/platforms
build-sdk/sdk/ohos_arm64/include/plugins
build-sdk/sdk/ohos_arm64/include/runtime
build-sdk/sdk/ohos_arm64/include/third_party
build-sdk/sdk/ohos_arm64/include/tools
build-sdk/sdk/ohos_arm64/include/verification
build-sdk/sdk/ohos_arm64/lib
build-sdk/sdk/ohos_arm64/lib/libarkaotmanager.so
build-sdk/sdk/ohos_arm64/lib/libarkassembler.so
build-sdk/sdk/ohos_arm64/lib/libarkbase.so
build-sdk/sdk/ohos_arm64/lib/libarkbytecodeopt.so
build-sdk/sdk/ohos_arm64/lib/libarkcompiler.so
build-sdk/sdk/ohos_arm64/lib/libarkdisassembler.so
build-sdk/sdk/ohos_arm64/lib/libarkfile.so
build-sdk/sdk/ohos_arm64/lib/libarkruntime.so
build-sdk/sdk/ohos_arm64/lib/libarkruntime_static.a
build-sdk/sdk/ohos_arm64/lib/libarkziparchive.so
build-sdk/sdk/ohos_arm64/lib/libc_secshared.so
build-sdk/sdk/ohos_arm64/lib/libdprof.a
build-sdk/sdk/ohos_arm64/lib/libe2p_test_plugin.so
build-sdk/sdk/ohos_arm64/lib/libes2panda-lib.so
build-sdk/sdk/ohos_arm64/lib/libes2panda-public.so
build-sdk/sdk/ohos_arm64/lib/libetsnative.so
build-sdk/sdk/ohos_arm64/lib/libets_interop_js_napi.so
build-sdk/sdk/ohos_arm64/lib/libhmicui18n.z.so
build-sdk/sdk/ohos_arm64/lib/libhmicuuc.z.so
build-sdk/sdk/ohos_arm64/lib/libinit_icu.a
build-sdk/sdk/ohos_arm64/plugins
build-sdk/sdk/ohos_arm64/plugins/ecmascript
build-sdk/sdk/package.json
build-sdk/sdk/tslinter
build-sdk/sdk/tslinter/README.md
build-sdk/sdk/tslinter/bin
build-sdk/sdk/tslinter/bin/tslinter.js
build-sdk/sdk/tslinter/dist
build-sdk/sdk/tslinter/dist/tslinter.js
build-sdk/sdk/tslinter/package.json
build-sdk/sdk/windows_host_tools
build-sdk/sdk/windows_host_tools/bin
build-sdk/sdk/windows_host_tools/bin/ark_link.exe
build-sdk/sdk/windows_host_tools/bin/es2panda.exe' >"$EXPECTED"
diff "$ACTUAL" "$EXPECTED"

rm "$ACTUAL" "$EXPECTED"
