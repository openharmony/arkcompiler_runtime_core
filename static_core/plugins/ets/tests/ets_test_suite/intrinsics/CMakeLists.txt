# Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

set(intrinsics_tests
    isNaN
    isFinite
    isInteger
    isSafeInteger
    string_equals
    string_length
    string_isempty
    string_from_string
    string_from_chars
    string_substring
    string_getchars
    string_getbytes
    string_hashcode
    string_concat
    string_trim
    string_startswith
    string_endswith
    string_indexof
    string_repeat
    string_compare_to
    char_isuppercase
    abs
    min
    max
    round
    floor
    ceil
    trunc
    stringbuilder_oom
    stringbuilder
)

if (NOT PANDA_TARGET_ARM32)
    # need coroutine support
    list(APPEND intrinsics_tests to_string_cache)
endif()

set(intrinsics_tests_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(intrinsics_tests_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(ets_test_suite_intrinsics)

foreach(test ${intrinsics_tests})
    set(test_out_dir "${intrinsics_tests_out_dir}/${test}")

    set(test_in "${intrinsics_tests_in_dir}/${test}.ets")
    set(target ets_test_suite_intrinsics_${test})
    set(extra_options "")

    # disable OSR for the tests throwing exceptions
    if ("${test}" STREQUAL "stringbuilder_oom")
        set(extra_options RUNTIME_EXTRA_OPTIONS "--compiler-enable-osr=false")
    endif()

    if ("${test}" STREQUAL "stringbuilder" OR
        "${test}" STREQUAL "string_compare_to")
        run_int_jit_aot_ets_code_foreach_gc(ets_test_suite_intrinsics ${test_in} ${test_out_dir} ${target})
    else()
        run_int_jit_aot_ets_code(${test_in} ${test_out_dir} ${target} ${extra_options})
        add_dependencies(ets_test_suite_intrinsics ${target})
    endif()
endforeach()
