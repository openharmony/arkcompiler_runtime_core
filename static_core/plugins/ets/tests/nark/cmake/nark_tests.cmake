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

add_custom_target(nark_tests COMMENT "Common target to run NARK ETS tests")

# Add gtest-based tests to nark_tests target.
#
# Example usage:
#   nark_add_gtest(test_name
#     CPP_SOURCES
#       tests/unit1_test.cpp
#       tests/unit2_test.cpp
#     ETS_SOURCES
#       tests/unit1_test.sts
#     LIBRARIES
#       lib_target1
#       lib_target2
#   )
function(nark_add_gtest TARGET)
    cmake_parse_arguments(
        ARG # give prefix `ARG` to each argument
        ""
        ""
        "CPP_SOURCES;ETS_SOURCES;LIBRARIES"
        ${ARGN}
    )

    if(NOT DEFINED ARG_CPP_SOURCES)
        message(FATAL_ERROR "CPP_SOURCES is not defined")
    endif()

    if(DEFINED ARG_ETS_SOURCES)
        set(TARGET_GTEST_PACKAGE ${TARGET}_gtest_package)
        panda_ets_package_gtest(${TARGET_GTEST_PACKAGE}
            ETS_SOURCES ${ARG_ETS_SOURCES}
        )
        set(NARK_GTEST_ABC_PATH "NARK_GTEST_ABC_PATH=${PANDA_BINARY_ROOT}/abc-gtests/${TARGET_GTEST_PACKAGE}.zip")
    endif()

    # Add launcher <${TARGET}_gtests> target
    set(NATIVE_TESTS_DIR "${PANDA_BINARY_ROOT}/tests/native")
    panda_ets_add_gtest(
        NAME ${TARGET}
        NO_CORES
        CUSTOM_PRERUN_ENVIRONMENT
            "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
            "${NARK_GTEST_ABC_PATH}"

        SOURCES ${ARG_CPP_SOURCES}
        LIBRARIES ${ARG_LIBRARIES} nark_gtest arkruntime
        SANITIZERS ${PANDA_SANITIZERS_LIST}
        TSAN_EXTRA_OPTIONS ${ARG_TSAN_EXTRA_OPTIONS}
        DEPS_TARGETS etsstdlib ${TARGET_GTEST_PACKAGE}
        TEST_RUN_DIR ${NATIVE_TESTS_DIR}
        OUTPUT_DIRECTORY ${NATIVE_TESTS_DIR}
    )

    add_dependencies(nark_tests ${TARGET}_gtests)
endfunction(nark_add_gtest)
