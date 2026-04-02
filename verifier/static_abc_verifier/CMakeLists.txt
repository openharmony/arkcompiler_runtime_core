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

cmake_minimum_required(VERSION 3.14)
project(static_abc_verifier CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---------- optional: link against pre-built libarkfile ----------
option(USE_ARKFILE "Link against libarkfile for deep structural verification" OFF)
set(ARKFILE_ROOT "" CACHE PATH "Root of static_core source tree (for headers)")
set(ARKFILE_LIB_DIR "" CACHE PATH "Directory containing pre-built libarkfile.so etc.")

set(VERIFIER_CORE_SOURCES
    src/abc_file.cpp
    src/verifier.cpp
    src/verify_api.cpp
)

set(ARKFILE_COMPILE_DEFS "")
set(ARKFILE_INCLUDE_DIRS "")
set(ARKFILE_LIBRARIES "")

if(USE_ARKFILE)
    if(NOT ARKFILE_ROOT)
        get_filename_component(ARKFILE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../static_core" ABSOLUTE)
    endif()
    if(NOT ARKFILE_LIB_DIR)
        set(ARKFILE_LIB_DIR "${ARKFILE_ROOT}/build/lib")
    endif()

    set(ARKFILE_BUILD_DIR "${ARKFILE_ROOT}/build")

    list(APPEND ARKFILE_COMPILE_DEFS
        STATIC_ABC_VERIFIER_USE_ARKFILE
        PANDA_TARGET_UNIX
        PANDA_TARGET_LINUX
        PANDA_TARGET_64
    )
    list(APPEND ARKFILE_INCLUDE_DIRS
        "${ARKFILE_ROOT}"
        "${ARKFILE_BUILD_DIR}"
        "${ARKFILE_BUILD_DIR}/libarkfile/include"
        "${ARKFILE_BUILD_DIR}/libarkbase/generated"
        "${ARKFILE_ROOT}/third_party/utils_native/base/include"
    )
    list(APPEND ARKFILE_LIBRARIES
        "${ARKFILE_LIB_DIR}/libarkfile.so"
        "${ARKFILE_LIB_DIR}/libarkbase.so"
        "${ARKFILE_LIB_DIR}/libarkziparchive.so"
        "${ARKFILE_LIB_DIR}/libc_secshared.so"
    )

    list(APPEND VERIFIER_CORE_SOURCES src/verifier_deep.cpp)
    message(STATUS "USE_ARKFILE=ON  root=${ARKFILE_ROOT}  lib=${ARKFILE_LIB_DIR}")
else()
    message(STATUS "USE_ARKFILE=OFF (basic structural checks only)")
endif()

# helper: apply arkfile settings to a target
function(apply_arkfile_settings TARGET_NAME)
    if(USE_ARKFILE)
        target_compile_definitions(${TARGET_NAME} PRIVATE ${ARKFILE_COMPILE_DEFS})
        target_include_directories(${TARGET_NAME} PRIVATE ${ARKFILE_INCLUDE_DIRS})
        target_link_libraries(${TARGET_NAME} PRIVATE ${ARKFILE_LIBRARIES})
    endif()
endfunction()

# ---------- static library ----------
add_library(static_abc_verifier_static STATIC ${VERIFIER_CORE_SOURCES})
target_include_directories(static_abc_verifier_static PUBLIC src)
target_compile_definitions(static_abc_verifier_static PUBLIC
    STATIC_ABC_VERIFIER_SKIP_VERSION_CHECK
)
set_target_properties(static_abc_verifier_static PROPERTIES
    OUTPUT_NAME "staticabcverifier"
)
apply_arkfile_settings(static_abc_verifier_static)

# ---------- shared library ----------
add_library(static_abc_verifier_shared SHARED ${VERIFIER_CORE_SOURCES})
target_include_directories(static_abc_verifier_shared PUBLIC src)
target_compile_definitions(static_abc_verifier_shared PUBLIC
    STATIC_ABC_VERIFIER_SKIP_VERSION_CHECK
)
set_target_properties(static_abc_verifier_shared PROPERTIES
    OUTPUT_NAME "staticabcverifier"
    VERSION 1.0.0
    SOVERSION 1
)
apply_arkfile_settings(static_abc_verifier_shared)

# ---------- CLI executable ----------
add_executable(static_abc_verifier_cli src/main.cpp)
target_link_libraries(static_abc_verifier_cli PRIVATE static_abc_verifier_static)
set_target_properties(static_abc_verifier_cli PROPERTIES
    OUTPUT_NAME "static_abc_verifier"
)

# ---------- JNI shared library ----------
option(BUILD_JNI "Build JNI shared library" ON)
if(BUILD_JNI)
    find_package(JNI)
    if(JNI_FOUND)
        add_library(static_abc_verifier_jni SHARED
            jni/static_abc_verifier_jni.cpp
            ${VERIFIER_CORE_SOURCES}
        )
        target_include_directories(static_abc_verifier_jni PRIVATE
            src
            ${JNI_INCLUDE_DIRS}
        )
        target_compile_definitions(static_abc_verifier_jni PUBLIC
            STATIC_ABC_VERIFIER_SKIP_VERSION_CHECK
        )
        set_target_properties(static_abc_verifier_jni PROPERTIES
            OUTPUT_NAME "staticabcverifier_jni"
        )
        apply_arkfile_settings(static_abc_verifier_jni)
    else()
        message(WARNING "JNI not found, skipping JNI library build")
    endif()
endif()

# ---------- Tests ----------
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        set(GTEST_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/googletest")
        if(EXISTS "${GTEST_LOCAL_DIR}/CMakeLists.txt")
            set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
            add_subdirectory("${GTEST_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/gtest_subdir" EXCLUDE_FROM_ALL)
        else()
            include(FetchContent)
            set(FETCHCONTENT_QUIET OFF)
            FetchContent_Declare(
                gtest_src
                URL https://gitee.com/mirrors/googletest/repository/archive/v1.14.0.tar.gz
                DOWNLOAD_EXTRACT_TIMESTAMP ON
            )
            set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
            FetchContent_MakeAvailable(gtest_src)
        endif()
    endif()

    enable_testing()
    add_subdirectory(tests)
endif()

# ---------- Install ----------
install(TARGETS static_abc_verifier_shared static_abc_verifier_static static_abc_verifier_cli
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(FILES src/verify_api.h DESTINATION include)
