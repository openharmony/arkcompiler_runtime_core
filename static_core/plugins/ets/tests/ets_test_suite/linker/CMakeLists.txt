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

set(linker_tests
    default_method1
    # #17626: enable negative/.abc integrity tests for runtime class linker
    # default_method2 # negative
    # default_method3 # negative
    # default_method4 # negative
    # end #17626
    implements_basic
    implements_variance
    override_abstract
    override_basic
    variance
)

set(linker_tests_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(linker_tests_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(ets_test_suite_linker)

foreach(test ${linker_tests})
    set(test_out_dir "${linker_tests_out_dir}/${test}")

    set(test_in "${linker_tests_in_dir}/${test}.sts")
    set(target ets_test_suite_linker_${test})

    run_int_jit_aot_ets_code(${test_in} ${test_out_dir} ${target})
    add_dependencies(ets_test_suite_linker
        ${target}-ets-jit
        ${target}-ets-int)
    if (NOT CMAKE_CROSSCOMPILING)
        add_dependencies(ets_test_suite_linker ${target}-ets-aot)
    endif()
endforeach()
