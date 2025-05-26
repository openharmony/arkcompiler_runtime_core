# Copyright (c) 2025 Huawei Device Co., Ltd.
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

add_custom_target(ets_interop_js_nodevm_gtests COMMENT "Run ets_interop_js Gtests on NodeVM")
add_dependencies(ets_gtests ets_interop_js_nodevm_gtests)
add_dependencies(ets_interop_tests ets_interop_js_nodevm_gtests)

add_custom_target(ets_interop_js_tests_nodevm COMMENT "Run ets_interop_js tests on NodeVM")
add_dependencies(ets_tests ets_interop_js_tests_nodevm)
add_dependencies(ets_interop_tests ets_interop_js_tests_nodevm)

# Add Gtest-based tests to ets_interop_js_nodevm_gtests target.
#
# Example usage:
#   panda_ets_interop_js_nodevm_gtest(test_name
#     CPP_SOURCES
#       tests/unit1_test.cpp
#       tests/unit2_test.cpp
#     ETS_SOURCES
#       tests/unit1_test.ets
#       tests/unit2_test.ets
#     LIBRARIES
#       lib_target1
#       lib_target2
#     PACKAGE_NAME
#       unit1_test 
#     ETS_CONFIG
#       path/to/arktsconfig.json
#   )
function(panda_ets_interop_js_nodevm_gtest TARGET)
    # Parse arguments
    cmake_parse_arguments(
        ARG
        ""
        "ETS_CONFIG;PACKAGE_NAME"
        "CPP_SOURCES;ETS_SOURCES;LIBRARIES"
        ${ARGN}
    )

    panda_ets_interop_js_plugin(${TARGET}
        SOURCES ${ARG_CPP_SOURCES}
        LIBRARIES ets_interop_js_gtest ets_interop_js_napi ${ARG_LIBRARIES}
        LIBRARY_OUTPUT_DIRECTORY "${INTEROP_TESTS_DIR}/lib/module"
        OUTPUT_SUFFIX ".node"
    )

    set(TARGET_GTEST_PACKAGE ${TARGET}_gtest_package)
    panda_ets_package_gtest(${TARGET_GTEST_PACKAGE}
        ETS_SOURCES ${ARG_ETS_SOURCES}
        ETS_CONFIG ${ARG_ETS_CONFIG}
    )
    add_dependencies(${TARGET} ${TARGET_GTEST_PACKAGE})
    # if not set PACKAGE_NAME, using first ets file as its name;
    set(ETS_SOURCES_NUM)
    list(LENGTH ARG_ETS_SOURCES ETS_SOURCES_NUM)
    if(NOT DEFINED ARG_PACKAGE_NAME AND ${ETS_SOURCES_NUM} EQUAL 1)
        list(GET ARG_ETS_SOURCES 0 PACKATE_FILE)
        get_filename_component(ARG_PACKAGE_NAME ${PACKATE_FILE} NAME_WE)
    elseif(NOT DEFINED ARG_PACKAGE_NAME)
        message("Please provide PACKAGE_NAME for ${TARGET}")
    endif()
    # Add launcher <${TARGET}_gtests> target
    panda_ets_add_gtest(
        NAME ${TARGET}
        NO_EXECUTABLE
        NO_CORES
        CUSTOM_PRERUN_ENVIRONMENT
            "NODE_PATH=${PANDA_BINARY_ROOT}:${CMAKE_CURRENT_SOURCE_DIR}"
            "JS_ABC_OUTPUT_PATH=${CMAKE_CURRENT_BINARY_DIR}"
            "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
            "ARK_ETS_INTEROP_JS_GTEST_ABC_PATH=${PANDA_BINARY_ROOT}/abc-gtests/${TARGET_GTEST_PACKAGE}.zip"
            "ARK_ETS_INTEROP_JS_GTEST_SOURCES=${CMAKE_CURRENT_SOURCE_DIR}"
            "ARK_ETS_INTEROP_JS_GTEST_DIR=${INTEROP_TESTS_DIR}"
            "FIXED_ISSUES=${FIXED_ISSUES}"
            "PACKAGE_NAME=${ARG_PACKAGE_NAME}"
        LAUNCHER ${NODE_BINARY} gtest_launcher_nodevm.js ${TARGET}
        DEPS_TARGETS ${TARGET} ets_interop_js_gtest_launcher
        TEST_RUN_DIR ${INTEROP_TESTS_DIR}
        OUTPUT_DIRECTORY ${INTEROP_TESTS_DIR}
    )

    add_dependencies(ets_interop_js_nodevm_gtests ${TARGET}_gtests)
endfunction(panda_ets_interop_js_nodevm_gtest)

function(panda_ets_interop_js_test_nodevm TARGET)
    # Parse arguments
    cmake_parse_arguments(
        ARG
        ""
        "JS_LAUNCHER;ETS_CONFIG"
        "NODE_OPTIONS;ABC_FILE;CPP_SOURCES;ETS_SOURCES;LAUNCHER_ARGS;ETS_VERIFICATOR_ERRORS"
        ${ARGN}
    )

    set(TARGET_TEST_PACKAGE ${TARGET}_test_package)
    panda_ets_package(${TARGET_TEST_PACKAGE}
        ABC_FILE ${ARG_ABC_FILE}
        ETS_SOURCES ${ARG_ETS_SOURCES}
        ETS_CONFIG ${ARG_ETS_CONFIG}
    )
    set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_interop_js_output.txt")

    add_custom_target(${TARGET}
        COMMAND "/usr/bin/env" "MODULE_PATH=${CMAKE_BINARY_DIR}/lib/module" "ARK_ETS_INTEROP_JS_GTEST_ABC_PATH=${PANDA_BINARY_ROOT}/abc/${TARGET_TEST_PACKAGE}.zip"
            "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc" ${NODE_BINARY} ${ARG_NODE_OPTIONS} ${ARG_JS_LAUNCHER} ${ARG_LAUNCHER_ARGS} > ${OUTPUT_FILE} 2>&1
            || (cat ${OUTPUT_FILE} && false)
            DEPENDS ${ARG_JS_LAUNCHER} ${TARGET_TEST_PACKAGE} ets_interop_js_napi
    )

    add_dependencies(ets_interop_js_tests_nodevm ${TARGET})
endfunction(panda_ets_interop_js_test_nodevm)
