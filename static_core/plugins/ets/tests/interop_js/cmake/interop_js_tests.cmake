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


add_custom_target(ets_interop_tests COMMENT "Common target to run ETS interop tests")

add_custom_target(ets_interop_js_gtests COMMENT "Run ets_interop_js Google tests")
add_dependencies(ets_gtests ets_interop_js_gtests)
add_dependencies(ets_interop_tests ets_interop_js_gtests)

add_custom_target(ets_interop_js_tests COMMENT "Run ets_interop_js tests")
add_dependencies(ets_tests ets_interop_js_tests)
add_dependencies(ets_interop_tests ets_interop_js_tests)


# Add Googletest-based tests to ets_interop_js_gtests target.
#
# Example usage:
#   panda_ets_interop_js_gtest(test_name
#     CPP_SOURCES
#       tests/unit1_test.cpp
#       tests/unit2_test.cpp
#     ETS_SOURCES
#       tests/unit1_test.sts
#       tests/unit2_test.sts
#     LIBRARIES
#       lib_target1
#       lib_target2
#     ETS_CONFIG
#       path/to/arktsconfig.json
#   )
function(panda_ets_interop_js_gtest TARGET)
    # Parse arguments
    cmake_parse_arguments(
        ARG
        ""
        "ETS_CONFIG"
        "CPP_SOURCES;ETS_SOURCES;LIBRARIES"
        ${ARGN}
    )

    set(INTEROP_TESTS_DIR "${PANDA_BINARY_ROOT}/tests/ets_interop_js")
    panda_ets_interop_js_plugin(${TARGET}
        SOURCES ${ARG_CPP_SOURCES}
        LIBRARIES ets_interop_js_gtest ets_interop_js_napi ${ARG_LIBRARIES}
        LIBRARY_OUTPUT_DIRECTORY "${INTEROP_TESTS_DIR}/lib/module"
    )

    set(TARGET_GTEST_PACKAGE ${TARGET}_gtest_package)
    panda_ets_package_gtest(${TARGET_GTEST_PACKAGE}
        ETS_SOURCES ${ARG_ETS_SOURCES}
        ETS_CONFIG ${ARG_ETS_CONFIG}
    )
    add_dependencies(${TARGET} ${TARGET_GTEST_PACKAGE})

    # Add launcher <${TARGET}_gtests> target
    panda_ets_add_gtest(
        NAME ${TARGET}
        NO_EXECUTABLE
        NO_CORES
        CUSTOM_PRERUN_ENVIRONMENT
            "NODE_PATH=${PANDA_BINARY_ROOT}:${CMAKE_CURRENT_SOURCE_DIR}"
            "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
            "ARK_ETS_INTEROP_JS_GTEST_ABC_PATH=${PANDA_BINARY_ROOT}/abc-gtests/${TARGET_GTEST_PACKAGE}.zip"
            "ARK_ETS_INTEROP_JS_GTEST_SOURCES=${CMAKE_CURRENT_SOURCE_DIR}"
            "ARR_ETS_INTEROP_JS_GTEST_DIR=${INTEROP_TESTS_DIR}"
            "FIXED_ISSUES=${FIXED_ISSUES}"
        LAUNCHER ${NODE_BINARY} gtest_launcher.js ${TARGET}
        DEPS_TARGETS ${TARGET} ets_interop_js_gtest_launcher
        TEST_RUN_DIR ${INTEROP_TESTS_DIR}
        OUTPUT_DIRECTORY ${INTEROP_TESTS_DIR}
    )

    add_dependencies(ets_interop_js_gtests ${TARGET}_gtests)
endfunction(panda_ets_interop_js_gtest)

function(panda_ets_interop_js_test TARGET)
    # Parse arguments
    cmake_parse_arguments(
        ARG
        ""
        "JS_LAUNCHER;ETS_CONFIG"
        "ABC_FILE;CPP_SOURCES;ETS_SOURCES;LAUNCHER_ARGS;ETS_VERIFICATOR_ERRORS"
        ${ARGN}
    )

    set(INTEROP_TESTS_DIR "${PANDA_BINARY_ROOT}/tests/ets_interop_js")

    set(TARGET_TEST_PACKAGE ${TARGET}_test_package)
    panda_ets_package(${TARGET_TEST_PACKAGE}
        ABC_FILE ${ARG_ABC_FILE}
        ETS_SOURCES ${ARG_ETS_SOURCES}
        ETS_CONFIG ${ARG_ETS_CONFIG}
    )
    set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_interop_js_output.txt")

    add_custom_target(${TARGET}
        COMMAND "/usr/bin/env" "MODULE_PATH=${CMAKE_BINARY_DIR}/lib/module" "ARK_ETS_INTEROP_JS_GTEST_ABC_PATH=${PANDA_BINARY_ROOT}/abc/${TARGET_TEST_PACKAGE}.zip"
            "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc" ${NODE_BINARY} ${ARG_JS_LAUNCHER} ${ARG_LAUNCHER_ARGS} > ${OUTPUT_FILE} 2>&1
            || (cat ${OUTPUT_FILE} && false)
            DEPENDS ${ARG_JS_LAUNCHER} ${TARGET_TEST_PACKAGE} ets_interop_js_napi
    )

    add_dependencies(ets_interop_js_tests ${TARGET})
endfunction(panda_ets_interop_js_test)

function(gen_file)
    cmake_parse_arguments(
        ARG
        ""
        "TARGET;TEMPLATE;OUTPUT;GENERATOR"
        "REQUIRES"
        ${ARGN}
    )

    if (NOT DEFINED ARG_GENERATOR)
        set(ARG_GENERATOR "${PANDA_ETS_PLUGIN_SOURCE}/tests/interop_js/gen_file.rb")
    endif()

    string(REPLACE ";" "," REQUIRE_STR "${ARG_REQUIRES}")

    add_custom_command(OUTPUT ${ARG_OUTPUT}
        COMMAND ${ARG_GENERATOR} ${ARG_TEMPLATE} ${ARG_OUTPUT} ${REQUIRE_STR}
        DEPENDS ${ARG_GENERATOR} ${ARG_TEMPLATE} ${ARG_REQUIRES}
    )

    add_custom_target(${ARG_TARGET}
        DEPENDS ${ARG_OUTPUT}
    )

    add_dependencies(panda_gen_files ${ARG_TARGET})
endfunction(gen_file)
