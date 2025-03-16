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

set(taskpool_test_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(taskpool_test_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(ets_test_suite_taskpool)

file(GLOB test_files "${taskpool_test_in_dir}/*.ets")
foreach(filename ${test_files})
    get_filename_component(test_file "${filename}" NAME)
    get_filename_component(test_name "${test_file}" NAME_WE)
    if (NOT "${test_file}" STREQUAL "group_tasks.ets" OR NOT PANDA_ENABLE_ADDRESS_SANITIZER OR NOT PANDA_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER)
        add_ets_test(ets_test_suite_taskpool
                    SKIP_ARM32_COMPILER
                    FILE ${test_file}
                    TEST_NAME "taskpool_${test_name}"
                    MODE "INT" "JIT" "AOT"
        )
    endif()
endforeach()
