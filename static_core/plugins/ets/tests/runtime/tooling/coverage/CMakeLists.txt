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

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

function(add_coverage_script_test test_name)
    set(PROJECT_TARGET_NAME test-coverage-script-${test_name})

    add_custom_target(${PROJECT_TARGET_NAME}
        COMMENT "Running coverage script test: ${test_name}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        COMMAND CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} PANDA_BINARY_ROOT=${PANDA_BINARY_ROOT} python3 test_coverage.py
        VERBATIM
    )

    add_dependencies(${PROJECT_TARGET_NAME} es2panda ark etsstdlib ark_disasm)
    add_dependencies(ets_tests ${PROJECT_TARGET_NAME})
endfunction()

add_coverage_script_test(host_output)
