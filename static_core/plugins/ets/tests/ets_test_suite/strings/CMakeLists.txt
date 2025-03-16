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

set(strings_tests
    string_api
    string_compare_to
)

set(strings_tests_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(strings_tests_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(ets_test_suite_strings)

foreach(test ${strings_tests})
    set(test_out_dir "${strings_tests_out_dir}/${test}")

    set(test_in "${strings_tests_in_dir}/${test}.sts")
    set(target ets_test_suite_strings_${test})

    if (PANDA_TARGET_ARM32)
        # failure in regalloc, see #13413
        set(extra_options RUNTIME_EXTRA_OPTIONS "--compiler-inlining-blacklist=std.core.ETSGLOBAL::copyTo,std.core.String::codePointAt" "--compiler-regex='(?!std.core.ETSGLOBAL::copyTo)&(?!std.core.String::codePointAt).*'" "--compiler-ignore-failures=true")
    endif()

    run_int_jit_aot_ets_code(${test_in} ${test_out_dir} ${target} ${extra_options})
    add_dependencies(ets_test_suite_strings
        ${target}-ets-jit
        ${target}-ets-int)
    if (NOT CMAKE_CROSSCOMPILING)
        add_dependencies(ets_test_suite_strings ${target}-ets-aot)
    endif()
endforeach()
