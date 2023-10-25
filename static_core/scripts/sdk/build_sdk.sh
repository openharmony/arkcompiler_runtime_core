#!/usr/bin/env bash
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

set -eo pipefail

SCRIPT_DIR="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"
ARK_ROOT="$SCRIPT_DIR/../.."

BUILD_TYPE_RELEASE="Release"
BUILD_TYPE_FAST_VERIFY="FastVerify"
BUILD_TYPE_DEBUG="Debug"

OHOS_SDK_NATIVE="$(realpath "$1")"
SDK_BUILD_ROOT="$(realpath "$2")"
PANDA_SDK_BUILD_TYPE="${3:-"$BUILD_TYPE_RELEASE"}"

usage() {
    echo "$(basename "${BASH_SOURCE[0]}") /path/to/ohos/sdk/native /path/to/panda/sdk/destination build_type:[$BUILD_TYPE_RELEASE,$BUILD_TYPE_FAST_VERIFY,$BUILD_TYPE_DEBUG]"
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

PANDA_SDK_PATH="$SDK_BUILD_ROOT/sdk"

# Arguments: BUILD_DIR, CMAKE_ARGUMENTS, NINJA_TARGETS
build_panda() {
    local BUILD_DIR="$1"
    local CMAKE_ARGUMENTS="$2"
    local NINJA_TARGETS="$3"

    local PRODUCT_BUILD="OFF"
    if [ "$PANDA_SDK_BUILD_TYPE" = "$BUILD_TYPE_RELEASE" ]; then
        PRODUCT_BUILD="ON"
    fi

    CONCURRENCY=${NPROC_PER_JOB:=$(nproc)}
    COMMONS_CMAKE_ARGS="\
        -GNinja \
        -S$ARK_ROOT \
        -DCMAKE_BUILD_TYPE=$PANDA_SDK_BUILD_TYPE \
        -DPANDA_PRODUCT_BUILD=$PRODUCT_BUILD \
        -DPANDA_WITH_ECMASCRIPT=ON \
        -DPANDA_WITH_ETS=ON \
        -DPANDA_WITH_JAVA=OFF \
        -DPANDA_WITH_ACCORD=OFF \
        -DPANDA_WITH_CANGJIE=OFF"

    cmake -B"$BUILD_DIR" ${COMMONS_CMAKE_ARGS[@]} $CMAKE_ARGUMENTS
    ninja -C"$BUILD_DIR" -j${CONCURRENCY} $NINJA_TARGETS
}

# Arguments: SRC, DST, FILE_LIST, INCLUDE_PATTERN
copy_into_sdk() {
    local SRC="$1"
    local DST="$2"
    local FILE_LIST="$3"
    local INCLUDE_PATTERN="$4"
    local EXCLUDE_PATTERN='\(/tests/\|/test/\)'

    # Below construction (cd + find + cp --parents) is used to copy
    # all files listed in FILE_LIST with their relative paths
    # Example: cd /path/to/panda && cp --parents runtime/include/cframe.h /dst
    # Result: /dst/runtime/include/cframe.h
    mkdir -p "$DST"
    cd "$SRC"
    for FILE in $(cat "$FILE_LIST"); do
        for F in $(find "$FILE" -type f | grep "$INCLUDE_PATTERN" | grep -v "$EXCLUDE_PATTERN"); do
            cp --parents "$F" "$DST"
        done
    done
}

linux_tools() {
    echo "> Building linux tools..."
    local LINUX_BUILD_DIR="$SDK_BUILD_ROOT/linux_host_tools"
    local LINUX_CMAKE_ARGS=" \
        -DPANDA_CROSS_AARCH64_TOOLCHAIN_FILE=cmake/toolchain/cross-ohos-musl-aarch64.cmake \
        -DTOOLCHAIN_SYSROOT=$OHOS_SDK_NATIVE/sysroot \
        -DTOOLCHAIN_CLANG_ROOT=$OHOS_SDK_NATIVE/llvm \
        -DPANDA_WITH_ECMASCRIPT=ON"
    local LINUX_BUILD_TARGETS="ark ark_aot ark_disasm ark_link es2panda"
    build_panda "$LINUX_BUILD_DIR" "$LINUX_CMAKE_ARGS" "$LINUX_BUILD_TARGETS"
    copy_into_sdk "$LINUX_BUILD_DIR" "$PANDA_SDK_PATH/linux_host_tools" "$SCRIPT_DIR"/linux_host_tools.txt
}

windows_tools() {
    echo "> Building windows tools..."
    local WINDOWS_BUILD_DIR="$SDK_BUILD_ROOT/windows_host_tools"
    local WINDOWS_CMAKE_ARGS="-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/cross-clang-14-x86_64-w64-mingw32-static.cmake"
    local WINDOWS_BUILD_TARGETS="es2panda ark_link"
    build_panda "$WINDOWS_BUILD_DIR" "$WINDOWS_CMAKE_ARGS" "$WINDOWS_BUILD_TARGETS"
    copy_into_sdk "$WINDOWS_BUILD_DIR" "$PANDA_SDK_PATH/windows_host_tools" "$SCRIPT_DIR"/windows_host_tools.txt
}

ohos() {
    echo "> Building runtime for OHOS ARM64..."
    local OHOS_BUILD_DIR="$SDK_BUILD_ROOT/ohos_arm64"
    local TARGET_SDK_DIR="$PANDA_SDK_PATH/ohos_arm64"
    local TARGET_CMAKE_ARGS=" \
        -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/cross-ohos-musl-aarch64.cmake \
        -DTOOLCHAIN_SYSROOT=$OHOS_SDK_NATIVE/sysroot \
        -DTOOLCHAIN_CLANG_ROOT=$OHOS_SDK_NATIVE/llvm \
        -DPANDA_ETS_INTEROP_JS=ON \
        -DPANDA_WITH_ECMASCRIPT=ON"
    local OHOS_BUILD_TARGETS="ark ark_aot arkruntime arkassembler ets_interop_js_napi"
    build_panda "$OHOS_BUILD_DIR" "$TARGET_CMAKE_ARGS" "$OHOS_BUILD_TARGETS"
    copy_into_sdk "$OHOS_BUILD_DIR" "$TARGET_SDK_DIR" "$SCRIPT_DIR"/ohos_arm64.txt

    echo "> Copying headers into SDK..."
    local HEADERS_DST="$TARGET_SDK_DIR"/include
    # Source headers
    copy_into_sdk "$ARK_ROOT" "$HEADERS_DST" "$SCRIPT_DIR"/headers.txt '\(\.h$\|\.inl$\|\.inc$\)'
    # Generated headers
    copy_into_sdk "$OHOS_BUILD_DIR" "$HEADERS_DST" "$SCRIPT_DIR"/gen_headers.txt '\(\.h$\|\.inl$\|\.inc$\)'
    # Copy compiled etsstdlib into Panda SDK
    mkdir -p "$PANDA_SDK_PATH"/ets
    cp -r "$OHOS_BUILD_DIR"/plugins/ets/etsstdlib.abc "$PANDA_SDK_PATH"/ets
}

ts_linter() {
    echo "> Building tslinter..."
    local LINTER_ROOT="$ARK_ROOT/tools/es2panda/linter"
    (cd "$LINTER_ROOT" && npm install)
    local TGZ="$(ls "$LINTER_ROOT"/bundle/panda-tslinter*tgz)"
    mkdir -p "$PANDA_SDK_PATH"/tslinter
    tar -xf "$TGZ" -C "$PANDA_SDK_PATH"/tslinter
    mv "$PANDA_SDK_PATH"/tslinter/package/* "$PANDA_SDK_PATH"/tslinter/
    rm -rf "$PANDA_SDK_PATH"/tslinter/node_modules
    rm -rf "$PANDA_SDK_PATH"/tslinter/package

    # Clean up
    rm "$TGZ"
    rm "$LINTER_ROOT"/package-lock.json
    rm -rf "$LINTER_ROOT"/build
    rm -rf "$LINTER_ROOT"/bundle
    rm -rf "$LINTER_ROOT"/dist
    rm -rf "$LINTER_ROOT"/node_modules
}

ets_std_lib() {
    echo "> Copying ets std lib into SDK..."
    mkdir -p "$PANDA_SDK_PATH"/ets
    cp -r "$ARK_ROOT"/plugins/ets/stdlib "$PANDA_SDK_PATH"/ets
}

rm -rf "$PANDA_SDK_PATH"
ohos
linux_tools
windows_tools
ts_linter
ets_std_lib

echo "> Packing NPM package..."
cp "$SCRIPT_DIR"/package.json "$PANDA_SDK_PATH"
cd "$PANDA_SDK_PATH"
npm pack --pack-destination "$SDK_BUILD_ROOT"
