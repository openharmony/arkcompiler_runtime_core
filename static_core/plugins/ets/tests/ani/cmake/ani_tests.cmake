# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

add_custom_target(ani_tests COMMENT "Common target to run ANI ETS tests")

# Add gtest-based tests to ani_tests target.
#
# Example usage:
#   ani_add_gtest(test_name
#     ETS_CONFIG path/to/arktsconfig.json
#     CPP_SOURCES
#       tests/unit1_test.cpp
#       tests/unit2_test.cpp
#     ETS_SOURCES
#       tests/unit1_test.ets
#     LIBRARIES
#       lib_target1
#       lib_target2
#   )
function(ani_add_gtest TARGET)
    cmake_parse_arguments(
        ARG # give prefix `ARG` to each argument
        ""
        "ETS_CONFIG"
        "CPP_SOURCES;ETS_SOURCES;LIBRARIES"
        ${ARGN}
    )
    if(NOT DEFINED ARG_CPP_SOURCES)
        message(FATAL_ERROR "CPP_SOURCES is not defined")
    endif()
    if(DEFINED ARG_ETS_SOURCES)
        set(TARGET_GTEST_PACKAGE ${TARGET}_gtest_package)
        set(VERIFY_SOURCES true)
        # NOTE(dslynko, #24335) Disable verifier on arm32 qemu due to flaky OOM
        if(PANDA_TARGET_ARM32 AND PANDA_QEMU_BUILD)
            set(VERIFY_SOURCES false)
        endif()

        panda_ets_package_gtest(${TARGET_GTEST_PACKAGE}
            ETS_SOURCES ${ARG_ETS_SOURCES}
            ETS_CONFIG ${ARG_ETS_CONFIG}
            VERIFY_SOURCES ${VERIFY_SOURCES}
        )
        set(ANI_GTEST_ABC_PATH "ANI_GTEST_ABC_PATH=${PANDA_BINARY_ROOT}/abc-gtests/${TARGET_GTEST_PACKAGE}.zip")
    endif()

    # Add launcher <${TARGET}_gtests> target
    set(NATIVE_TESTS_DIR "${PANDA_BINARY_ROOT}/tests/native")
    panda_ets_add_gtest(
        NAME ${TARGET}
        NO_CORES
        CUSTOM_PRERUN_ENVIRONMENT
            "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
            "${ANI_GTEST_ABC_PATH}"

        SOURCES ${ARG_CPP_SOURCES}
        INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani
        LIBRARIES ${ARG_LIBRARIES} ani_gtest arkruntime
        SANITIZERS ${PANDA_SANITIZERS_LIST}
        TSAN_EXTRA_OPTIONS ${ARG_TSAN_EXTRA_OPTIONS}
        DEPS_TARGETS etsstdlib ${TARGET_GTEST_PACKAGE}
        TEST_RUN_DIR ${NATIVE_TESTS_DIR}
        OUTPUT_DIRECTORY ${NATIVE_TESTS_DIR}
    )

    add_dependencies(ani_tests ${TARGET}_gtests)
endfunction(ani_add_gtest)
