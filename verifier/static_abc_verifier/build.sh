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
MAVEN_TARBALL="apache-maven-${MAVEN_VERSION}-bin.tar.gz"
MAVEN_DIST_URLS=(
    "https://dlcdn.apache.org/maven/maven-3/${MAVEN_VERSION}/binaries/${MAVEN_TARBALL}"
    "https://mirrors.tuna.tsinghua.edu.cn/apache/maven/maven-3/${MAVEN_VERSION}/binaries/${MAVEN_TARBALL}"
    "https://mirrors.aliyun.com/apache/maven/maven-3/${MAVEN_VERSION}/binaries/${MAVEN_TARBALL}"
    "https://archive.apache.org/dist/maven/maven-3/${MAVEN_VERSION}/binaries/${MAVEN_TARBALL}"
)
JAR_NAME="static-abc-verifier-1.0.0.0.jar"

run_privileged() {
    if [[ "$(id -u)" -eq 0 ]]; then
        "$@"
    elif command -v sudo >/dev/null 2>&1; then
        sudo "$@"
    else
        return 1
    fi
}

install_maven_via_apt() {
    if ! command -v apt-get >/dev/null 2>&1; then
        return 1
    fi
    echo "=== Installing Maven via apt (recommended for speed) ===" >&2
    run_privileged apt-get update -qq || return 1
    DEBIAN_FRONTEND=noninteractive run_privileged apt-get install -y maven || return 1
    command -v mvn >/dev/null 2>&1
}

download_maven_toolchain() {
    local maven_home="${MAVEN_TOOLS_DIR}/apache-maven-${MAVEN_VERSION}"
    local mvn_cmd="${maven_home}/bin/mvn"
    if [[ -x "${mvn_cmd}" ]]; then
        return 0
    fi

    echo "=== Maven not found; downloading Apache Maven ${MAVEN_VERSION} ===" >&2
    mkdir -p "${MAVEN_TOOLS_DIR}"
    local tarball="${MAVEN_TOOLS_DIR}/${MAVEN_TARBALL}"
    rm -f "${tarball}"

    local url
    for url in "${MAVEN_DIST_URLS[@]}"; do
        echo "Trying mirror: ${url}" >&2
        if curl -fSL --connect-timeout 10 --retry 2 --retry-delay 1 \
                -o "${tarball}" "${url}"; then
            break
        fi
        rm -f "${tarball}"
    done

    if [[ ! -s "${tarball}" ]]; then
        echo "Failed to download Maven from all mirrors." >&2
        echo "Install manually: sudo apt-get install -y maven" >&2
        return 1
    fi

    tar -xzf "${tarball}" -C "${MAVEN_TOOLS_DIR}"
    rm -f "${tarball}"
    [[ -x "${mvn_cmd}" ]]
}

# Resolve Maven when publish-local or --use-maven needs it.
mvn_exec() {
    local mvn_cmd=""
    if command -v mvn >/dev/null 2>&1; then
        mvn_cmd="mvn"
    else
        local maven_home="${MAVEN_TOOLS_DIR}/apache-maven-${MAVEN_VERSION}"
        mvn_cmd="${maven_home}/bin/mvn"
        if [[ ! -x "${mvn_cmd}" ]]; then
            if [[ "${AUTO_INSTALL_DEPS}" == "1" ]] && install_maven_via_apt; then
                mvn_cmd="mvn"
            elif ! download_maven_toolchain; then
                exit 1
            fi
        fi
    fi
    "${mvn_cmd}" "$@"
}

ensure_javac() {
    if command -v javac >/dev/null 2>&1 && command -v jar >/dev/null 2>&1; then
        return 0
    fi
    if [[ "${AUTO_INSTALL_DEPS}" != "1" ]]; then
        echo "javac/jar not found. Install JDK: sudo apt-get install -y default-jdk" >&2
        exit 1
    fi
    echo "=== Installing JDK (javac/jar) via apt ===" >&2
    run_privileged apt-get update -qq
    DEBIAN_FRONTEND=noninteractive run_privileged apt-get install -y default-jdk
    command -v javac >/dev/null 2>&1 && command -v jar >/dev/null 2>&1
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
  jni-all         Build JNI shared libraries for both x86_64 and aarch64
  java-wrapper    Build Java verifier wrapper JAR via Maven (native .so must exist)
  publish-local   Build JNI for current platform and publish JAR to local Maven repository
  publish-multiarch
                  Build JNI for x86_64 and aarch64, then package multi-arch JAR
  clean           Remove all build artifacts

Options:
  --build-type         CMake build type: Debug|Release|RelWithDebInfo (default: Release)
  --use-arkfile        Enable deep verification via libarkfile (auto-detects paths)
  --skip-install-deps  Do not auto-install missing cross-compiler packages via apt
  --use-maven          Use Maven to build the Java wrapper (slower first run)
  --help               Show this help

Examples:
  ./build.sh native
  ./build.sh native --use-arkfile
  ./build.sh test
  ./build.sh test-deep
  ./build.sh jni --use-arkfile
  ./build.sh publish-local
  ./build.sh publish-multiarch
  ./build.sh java-wrapper
  ./build.sh native --build-type Debug
EOF
}

BUILD_TYPE="Release"
USE_ARKFILE="OFF"
AUTO_INSTALL_DEPS="1"
USE_MAVEN="0"
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
        --skip-install-deps)
            AUTO_INSTALL_DEPS="0"
            shift
            ;;
        --use-maven)
            USE_MAVEN="1"
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

MULTIARCH_TARGETS=(x86_64 aarch64)

cross_compiler_cc() {
    case "$1" in
        aarch64) echo "aarch64-linux-gnu-gcc" ;;
        x86_64)  echo "x86_64-linux-gnu-gcc" ;;
        *)       echo "" ;;
    esac
}

cross_compiler_cxx() {
    case "$1" in
        aarch64) echo "aarch64-linux-gnu-g++" ;;
        x86_64)  echo "x86_64-linux-gnu-g++" ;;
        *)       echo "" ;;
    esac
}

cross_compiler_packages() {
    case "$1" in
        aarch64) echo "gcc-aarch64-linux-gnu g++-aarch64-linux-gnu" ;;
        x86_64)  echo "gcc-x86-64-linux-gnu g++-x86-64-linux-gnu" ;;
        *)       echo "" ;;
    esac
}

has_cross_compiler() {
    local arch="$1"
    local cc cxx
    cc="$(cross_compiler_cc "${arch}")"
    cxx="$(cross_compiler_cxx "${arch}")"
    [[ -n "${cc}" && -n "${cxx}" ]] \
        && command -v "${cc}" >/dev/null 2>&1 \
        && command -v "${cxx}" >/dev/null 2>&1
}

install_cross_compiler() {
    local arch="$1"
    local -a packages=()
    read -ra packages <<< "$(cross_compiler_packages "${arch}")"

    if [[ ${#packages[@]} -eq 0 ]]; then
        echo "Unsupported architecture for cross-compilation: ${arch}" >&2
        return 1
    fi

    echo "=== Installing cross-compiler for ${arch}: ${packages[*]} ===" >&2

    if command -v apt-get >/dev/null 2>&1; then
        local -a apt_cmd=(apt-get)
        if [[ "$(id -u)" -ne 0 ]]; then
            if command -v sudo >/dev/null 2>&1; then
                apt_cmd=(sudo apt-get)
            else
                echo "Need root privileges to install: ${packages[*]}" >&2
                echo "Run: apt-get install -y ${packages[*]}" >&2
                return 1
            fi
        fi
        DEBIAN_FRONTEND=noninteractive "${apt_cmd[@]}" update -qq
        DEBIAN_FRONTEND=noninteractive "${apt_cmd[@]}" install -y "${packages[@]}"
        return 0
    fi

    if command -v dnf >/dev/null 2>&1; then
        local -a dnf_cmd=(dnf)
        if [[ "$(id -u)" -ne 0 ]]; then
            dnf_cmd=(sudo dnf)
        fi
        "${dnf_cmd[@]}" install -y "${packages[@]}"
        return 0
    fi

    if command -v yum >/dev/null 2>&1; then
        local -a yum_cmd=(yum)
        if [[ "$(id -u)" -ne 0 ]]; then
            yum_cmd=(sudo yum)
        fi
        "${yum_cmd[@]}" install -y "${packages[@]}"
        return 0
    fi

    echo "No supported package manager found (apt-get/dnf/yum)." >&2
    echo "Install manually: ${packages[*]}" >&2
    return 1
}

require_cross_compiler() {
    local arch="$1"
    local cc cxx pkg_hint

    cc="$(cross_compiler_cc "${arch}")"
    cxx="$(cross_compiler_cxx "${arch}")"
    pkg_hint="$(cross_compiler_packages "${arch}")"

    if [[ -z "${cc}" || -z "${cxx}" ]]; then
        echo "Unsupported architecture for cross-compilation: ${arch}" >&2
        exit 1
    fi

    if has_cross_compiler "${arch}"; then
        return 0
    fi

    if [[ "${AUTO_INSTALL_DEPS}" == "1" ]]; then
        install_cross_compiler "${arch}" || exit 1
        if has_cross_compiler "${arch}"; then
            return 0
        fi
        echo "Cross-compiler still not available for ${arch} after installation." >&2
        exit 1
    fi

    echo "Cross-compiler not found for ${arch}." >&2
    echo "Install: ${pkg_hint}" >&2
    echo "Or rerun with auto-install enabled (default): ./build.sh ${COMMAND:-publish-multiarch}" >&2
    exit 1
}

ensure_multiarch_build_deps() {
    local arch
    for arch in "${MULTIARCH_TARGETS[@]}"; do
        if [[ "${arch}" != "${HOST_ARCH}" ]]; then
            require_cross_compiler "${arch}"
        fi
    done
}

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
        require_cross_compiler "${arch}"
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
        require_cross_compiler "${arch}"
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

build_java_wrapper_javac() {
    local classes_dir jar_path java_src
    classes_dir="${JAVA_WRAPPER_DIR}/target/classes"
    jar_path="${JAVA_WRAPPER_DIR}/target/${JAR_NAME}"
    java_src="${JAVA_WRAPPER_DIR}/src/main/java/com/oh/ark/verifier"

    echo "=== Building Java verifier wrapper (javac fast path) ==="
    ensure_javac
    mkdir -p "${classes_dir}" "${JAVA_WRAPPER_DIR}/target"

    javac -encoding UTF-8 -d "${classes_dir}" "${java_src}/"*.java
    jar cf "${jar_path}" -C "${classes_dir}" . -C "${JAVA_WRAPPER_DIR}/src/main/resources" .

    echo "=== JAR built: ${jar_path} ==="
}

build_java_wrapper_maven() {
    echo "=== Building Java verifier wrapper (Maven) ==="
    cd "${JAVA_WRAPPER_DIR}"
    mvn_exec -q package -DskipTests -Dmaven.source.skip=true
    cd "${SCRIPT_DIR}"
    echo "=== JAR built: ${JAVA_WRAPPER_DIR}/target/${JAR_NAME} ==="
}

build_java_wrapper() {
    if [[ "${USE_MAVEN}" == "1" ]]; then
        build_java_wrapper_maven
    else
        build_java_wrapper_javac
    fi
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
    echo "    version:    1.0.0.0"
}

build_jni_all() {
    ensure_multiarch_build_deps
    local arch
    for arch in "${MULTIARCH_TARGETS[@]}"; do
        build_jni "${arch}"
    done
}

publish_multiarch() {
    echo "=== Building multi-arch JNI and packaging JAR ==="

    build_jni_all
    build_java_wrapper

    local jar_path
    jar_path="${JAVA_WRAPPER_DIR}/target/${JAR_NAME}"
    echo "=== Multi-arch JAR ready ==="
    echo "    jar:    ${jar_path}"
    echo "    native: linux-x86_64, linux-aarch64"
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
        ensure_multiarch_build_deps
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
    jni-all)
        build_jni_all
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
    publish-multiarch)
        publish_multiarch
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
