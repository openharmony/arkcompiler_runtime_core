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

add_custom_target(ets_native_gtests COMMENT "Run ets_native tests")
add_dependencies(ets_gtests ets_native_gtests)

# Add gtest-based tests to ets_native_gtests target.
#
# Example usage:
#   panda_ets_native_add_gtest(test_name
#     CPP_SOURCES
#       tests/unit1_test.cpp
#       tests/unit2_test.cpp
#     ETS_SOURCES
#       tests/unit1_test.sts
#     LIBRARIES
#       lib_target1
#       lib_target2
#   )
function(panda_ets_native_add_gtest TARGET)
    # Parse arguments
    cmake_parse_arguments(
        ARG
        ""
        ""
        "CPP_SOURCES;ETS_SOURCES;LIBRARIES"
        ${ARGN}
    )

    # Check args
    if(NOT DEFINED ARG_CPP_SOURCES)
        message(FATAL_ERROR "CPP_SOURCES is not defined")
    endif()

    if(DEFINED ARG_ETS_SOURCES)
        set(TARGET_GTEST_PACKAGE ${TARGET}_gtest_package)
        panda_ets_package_gtest(${TARGET_GTEST_PACKAGE}
            ETS_SOURCES ${ARG_ETS_SOURCES}
        )
        set(ARK_ETS_GTEST_ABC_PATH "ARK_ETS_GTEST_ABC_PATH=${PANDA_BINARY_ROOT}/abc-gtests/${TARGET_GTEST_PACKAGE}.zip")
    endif()

    # Add <${TARGET}_gtests> target
    set(NATIVE_TESTS_DIR "${PANDA_BINARY_ROOT}/tests/native")
    panda_ets_add_gtest(
        NO_CORES
        NAME ${TARGET}
        SOURCES ${ARG_CPP_SOURCES}
        LIBRARIES ${ARG_LIBRARIES} arkruntime
        SANITIZERS ${PANDA_SANITIZERS_LIST}
        TSAN_EXTRA_OPTIONS ${ARG_TSAN_EXTRA_OPTIONS}
        CUSTOM_PRERUN_ENVIRONMENT
            "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
            "${ARK_ETS_GTEST_ABC_PATH}"
        DEPS_TARGETS etsstdlib ${TARGET_GTEST_PACKAGE}
        TEST_RUN_DIR ${NATIVE_TESTS_DIR}
        OUTPUT_DIRECTORY ${NATIVE_TESTS_DIR}
    )

    add_dependencies(ets_native_gtests ${TARGET}_gtests)
endfunction(panda_ets_native_add_gtest)
