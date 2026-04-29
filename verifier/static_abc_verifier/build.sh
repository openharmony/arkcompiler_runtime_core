#!/bin/bash
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
JAVA_WRAPPER_DIR="${SCRIPT_DIR}/java-verifier-wrapper"
MAVEN_TOOLS_DIR="${SCRIPT_DIR}/.tools"
MAVEN_VERSION="3.9.11"
MAVEN_DIST_URL="https://archive.apache.org/dist/maven/maven-3/${MAVEN_VERSION}/binaries/apache-maven-${MAVEN_VERSION}-bin.tar.gz"

# Run Maven using the system mvn when available; otherwise download a binary distribution once.
mvn_exec() {
    local mvn_cmd
    if command -v mvn >/dev/null 2>&1; then
        mvn_cmd="mvn"
    else
        local maven_home="${MAVEN_TOOLS_DIR}/apache-maven-${MAVEN_VERSION}"
        mvn_cmd="${maven_home}/bin/mvn"
        if [[ ! -x "${mvn_cmd}" ]]; then
            echo "=== Maven not found in PATH; installing Apache Maven ${MAVEN_VERSION} under ${MAVEN_TOOLS_DIR} ===" >&2
            mkdir -p "${MAVEN_TOOLS_DIR}"
            local tarball="${MAVEN_TOOLS_DIR}/apache-maven-${MAVEN_VERSION}-bin.tar.gz"
            curl -fSL --progress-bar "${MAVEN_DIST_URL}" -o "${tarball}"
            tar -xzf "${tarball}" -C "${MAVEN_TOOLS_DIR}"
            rm -f "${tarball}"
        fi
    fi
    "${mvn_cmd}" "$@"
}

usage() {
    cat <<EOF
Usage: $(basename "$0") [COMMAND] [OPTIONS]

Commands:
  native          Build C++ native libraries (current platform)
  native-all      Build for both x86_64 and aarch64 (requires cross-compiler for aarch64)
  test            Build and run C++ unit tests (basic only)
  test-deep       Build and run all tests including deep checks (requires libarkfile)
  jni             Build JNI shared library (current platform)
  java-wrapper    Build Java verifier wrapper JAR via Maven (native .so must exist)
  publish-local   Build everything and publish JAR to local Maven repository
  clean           Remove all build artifacts

Options:
  --build-type    CMake build type: Debug|Release|RelWithDebInfo (default: Release)
  --use-arkfile   Enable deep verification via libarkfile (auto-detects paths)
  --help          Show this help

Examples:
  ./build.sh native
  ./build.sh native --use-arkfile
  ./build.sh test
  ./build.sh test-deep
  ./build.sh jni --use-arkfile
  ./build.sh publish-local
  ./build.sh java-wrapper
  ./build.sh native --build-type Debug
EOF
}

BUILD_TYPE="Release"
USE_ARKFILE="OFF"
STATIC_CORE_ROOT="$(cd "${SCRIPT_DIR}/../../static_core" && pwd)"

# Parse global options
while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --use-arkfile)
            USE_ARKFILE="ON"
            shift
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            break
            ;;
    esac
done

COMMAND="${1:-}"
shift || true

detect_arch() {
    local arch
    arch="$(uname -m)"
    case "$arch" in
        x86_64|amd64)   echo "x86_64" ;;
        aarch64|arm64)   echo "aarch64" ;;
        *)               echo "$arch" ;;
    esac
}

HOST_ARCH="$(detect_arch)"

build_native() {
    local arch="${1:-$HOST_ARCH}"
    local build_dir="${BUILD_DIR}/${arch}"
    local cmake_args=()

    echo "=== Building native libraries for ${arch} (${BUILD_TYPE}) ==="

    cmake_args+=(-DCMAKE_BUILD_TYPE="${BUILD_TYPE}")
    cmake_args+=(-DBUILD_TESTS=OFF)
    cmake_args+=(-DBUILD_JNI=OFF)

    if [[ "$USE_ARKFILE" == "ON" ]]; then
        cmake_args+=(-DUSE_ARKFILE=ON)
        cmake_args+=(-DARKFILE_ROOT="${STATIC_CORE_ROOT}")
    fi

    if [[ "$arch" != "$HOST_ARCH" ]]; then
        if [[ "$arch" == "aarch64" ]]; then
            cmake_args+=(-DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc)
            cmake_args+=(-DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++)
            cmake_args+=(-DCMAKE_SYSTEM_NAME=Linux)
            cmake_args+=(-DCMAKE_SYSTEM_PROCESSOR=aarch64)
        elif [[ "$arch" == "x86_64" ]]; then
            cmake_args+=(-DCMAKE_C_COMPILER=x86_64-linux-gnu-gcc)
            cmake_args+=(-DCMAKE_CXX_COMPILER=x86_64-linux-gnu-g++)
            cmake_args+=(-DCMAKE_SYSTEM_NAME=Linux)
            cmake_args+=(-DCMAKE_SYSTEM_PROCESSOR=x86_64)
        fi
    fi

    cmake -S "${SCRIPT_DIR}" -B "${build_dir}" "${cmake_args[@]}"
    cmake --build "${build_dir}" --parallel "$(nproc)"

    echo "=== Native build complete: ${build_dir} ==="
}

build_jni() {
    local arch="${1:-$HOST_ARCH}"
    local build_dir="${BUILD_DIR}/${arch}-jni"
    local cmake_args=()

    echo "=== Building JNI library for ${arch} (${BUILD_TYPE}) ==="

    cmake_args+=(-DCMAKE_BUILD_TYPE="${BUILD_TYPE}")
    cmake_args+=(-DBUILD_TESTS=OFF)
    cmake_args+=(-DBUILD_JNI=ON)

    if [[ "$USE_ARKFILE" == "ON" ]]; then
        cmake_args+=(-DUSE_ARKFILE=ON)
        cmake_args+=(-DARKFILE_ROOT="${STATIC_CORE_ROOT}")
    fi

    if [[ "$arch" != "$HOST_ARCH" ]]; then
        if [[ "$arch" == "aarch64" ]]; then
            cmake_args+=(-DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc)
            cmake_args+=(-DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++)
            cmake_args+=(-DCMAKE_SYSTEM_NAME=Linux)
            cmake_args+=(-DCMAKE_SYSTEM_PROCESSOR=aarch64)
        fi
    fi

    cmake -S "${SCRIPT_DIR}" -B "${build_dir}" "${cmake_args[@]}"
    cmake --build "${build_dir}" --parallel "$(nproc)"

    # Copy JNI .so to Java SDK resources
    local res_arch
    if [[ "$arch" == "x86_64" ]]; then
        res_arch="linux-x86_64"
    elif [[ "$arch" == "aarch64" ]]; then
        res_arch="linux-aarch64"
    else
        res_arch="linux-${arch}"
    fi

    local target_dir="${JAVA_WRAPPER_DIR}/src/main/resources/native/${res_arch}"
    mkdir -p "${target_dir}"
    cp "${build_dir}/libstaticabcverifier_jni.so" "${target_dir}/"

    echo "=== JNI library copied to ${target_dir} ==="
}

build_test() {
    local build_dir="${BUILD_DIR}/test"

    echo "=== Building and running C++ tests ==="

    cmake -S "${SCRIPT_DIR}" -B "${build_dir}" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_TESTS=ON \
        -DBUILD_JNI=OFF

    cmake --build "${build_dir}" --parallel "$(nproc)"

    cd "${build_dir}"
    ctest --output-on-failure --parallel "$(nproc)"
    cd "${SCRIPT_DIR}"

    echo "=== C++ tests complete ==="
}

build_test_deep() {
    local build_dir="${BUILD_DIR}/test-deep"

    echo "=== Building and running C++ tests with deep verification ==="

    cmake -S "${SCRIPT_DIR}" -B "${build_dir}" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_TESTS=ON \
        -DBUILD_JNI=OFF \
        -DUSE_ARKFILE=ON \
        -DARKFILE_ROOT="${STATIC_CORE_ROOT}"

    cmake --build "${build_dir}" --parallel "$(nproc)"

    export LD_LIBRARY_PATH="${STATIC_CORE_ROOT}/build/lib:${LD_LIBRARY_PATH:-}"
    cd "${build_dir}"
    ctest --output-on-failure --parallel "$(nproc)"
    cd "${SCRIPT_DIR}"

    echo "=== Deep C++ tests complete ==="
}

build_java_wrapper() {
    echo "=== Building Java verifier wrapper (Maven JAR) ==="
    cd "${JAVA_WRAPPER_DIR}"
    mvn_exec package -DskipTests
    cd "${SCRIPT_DIR}"
    echo "=== JAR built: ${JAVA_WRAPPER_DIR}/target/ ==="
}

publish_local() {
    echo "=== Building and publishing to local Maven repository ==="

    # Build JNI for current platform
    build_jni "${HOST_ARCH}"

    # Build Java wrapper JAR and install to local Maven repo
    cd "${JAVA_WRAPPER_DIR}"
    mvn_exec install
    cd "${SCRIPT_DIR}"

    echo "=== Published to local Maven repository ==="
    echo "    groupId:    com.oh.ark"
    echo "    artifactId: static-abc-verifier"
    echo "    version:    1.0.0-SNAPSHOT"
}

do_clean() {
    echo "=== Cleaning build artifacts ==="
    rm -rf "${BUILD_DIR}"
    rm -rf "${JAVA_WRAPPER_DIR}/target"
    rm -f "${JAVA_WRAPPER_DIR}/src/main/resources/native/linux-x86_64/"*.so
    rm -f "${JAVA_WRAPPER_DIR}/src/main/resources/native/linux-aarch64/"*.so
    echo "=== Clean complete ==="
}

case "${COMMAND}" in
    native)
        build_native "${HOST_ARCH}"
        ;;
    native-all)
        build_native "x86_64"
        build_native "aarch64"
        ;;
    test)
        build_test
        ;;
    test-deep)
        build_test_deep
        ;;
    jni)
        build_jni "${HOST_ARCH}"
        ;;
    java-wrapper)
        build_java_wrapper
        ;;
    java-sdk)
        echo "Note: command renamed from java-sdk to java-wrapper." >&2
        build_java_wrapper
        ;;
    publish-local)
        publish_local
        ;;
    clean)
        do_clean
        ;;
    "")
        usage
        exit 1
        ;;
    *)
        echo "Unknown command: ${COMMAND}"
        usage
        exit 1
        ;;
esac
