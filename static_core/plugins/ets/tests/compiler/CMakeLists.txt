# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

include(cmake/compiler_tests.cmake)

add_subdirectory(compiler_gtest)
add_subdirectory(codegen)
add_subdirectory(optimizer)
add_subdirectory(wrong_boot_context_test)

set(HOST_AOT_TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/host_aot)
set(HOST_AOT_TEST_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/host_aot)
set(HOST_AOT_TEST_RUNNER ${HOST_AOT_TEST_SOURCE_DIR}/runner.py)
set(HOST_AOT_SCRIPT ${PANDA_ROOT}/compiler/tools/paoc/host_aot_compile.py)
set(HOST_AOT_TEST_WORKDIR ${HOST_AOT_TEST_BINARY_DIR}/host_aot_workdir)
set(HOST_AOT_TEST_STAMP ${HOST_AOT_TEST_BINARY_DIR}/host_aot_test.stamp)

set(HOST_AOT_SINGLE_ETS ${HOST_AOT_TEST_SOURCE_DIR}/single_case.ets)
set(HOST_AOT_SINGLE_CONFIG ${HOST_AOT_TEST_SOURCE_DIR}/single_case_config.txt)

set(HOST_AOT_MULTI_FIRST_ETS ${HOST_AOT_TEST_SOURCE_DIR}/multi_first.ets)
set(HOST_AOT_MULTI_FIRST_CONFIG ${HOST_AOT_TEST_SOURCE_DIR}/multi_first_config.txt)
set(HOST_AOT_MULTI_SECOND_ETS ${HOST_AOT_TEST_SOURCE_DIR}/multi_second.ets)
set(HOST_AOT_MULTI_SECOND_CONFIG ${HOST_AOT_TEST_SOURCE_DIR}/multi_second_config.txt)

set(HOST_AOT_OVERLOAD_ETS ${HOST_AOT_TEST_SOURCE_DIR}/overload_case.ets)
set(HOST_AOT_OVERLOAD_CONFIG ${HOST_AOT_TEST_SOURCE_DIR}/overload_case_config.txt)

string(REPLACE ";" " " HOST_AOT_RUN_PREFIX "${PANDA_RUN_PREFIX}")

add_custom_target(compiler_host_aot_test
    COMMAND ${CMAKE_COMMAND} -E env
        PYTHONUNBUFFERED=1
        python3 ${HOST_AOT_TEST_RUNNER}
        --es2panda $<TARGET_FILE:es2panda>
        --es2panda-run-prefix "${HOST_AOT_RUN_PREFIX}"
        --ark-aot $<TARGET_FILE:ark_aot>
        --ark-aot-run-prefix "${HOST_AOT_RUN_PREFIX}"
        --host-aot-script ${HOST_AOT_SCRIPT}
        --ets-stdlib ${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
        --single-ets ${HOST_AOT_SINGLE_ETS}
        --single-config ${HOST_AOT_SINGLE_CONFIG}
        --multi-first-ets ${HOST_AOT_MULTI_FIRST_ETS}
        --multi-first-config ${HOST_AOT_MULTI_FIRST_CONFIG}
        --multi-second-ets ${HOST_AOT_MULTI_SECOND_ETS}
        --multi-second-config ${HOST_AOT_MULTI_SECOND_CONFIG}
        --overload-ets ${HOST_AOT_OVERLOAD_ETS}
        --overload-config ${HOST_AOT_OVERLOAD_CONFIG}
        --work-dir ${HOST_AOT_TEST_WORKDIR}
        --stamp ${HOST_AOT_TEST_STAMP}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS es2panda ark_aot etsstdlib
            ${HOST_AOT_TEST_RUNNER} ${HOST_AOT_SCRIPT}
            ${HOST_AOT_SINGLE_ETS} ${HOST_AOT_SINGLE_CONFIG}
            ${HOST_AOT_MULTI_FIRST_ETS} ${HOST_AOT_MULTI_FIRST_CONFIG}
            ${HOST_AOT_MULTI_SECOND_ETS} ${HOST_AOT_MULTI_SECOND_CONFIG}
            ${HOST_AOT_OVERLOAD_ETS} ${HOST_AOT_OVERLOAD_CONFIG}
    COMMENT "Running host_aot_compile integration test"
    VERBATIM
)

add_dependencies(compiler_tests compiler_host_aot_test)
