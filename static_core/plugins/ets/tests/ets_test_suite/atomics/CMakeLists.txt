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

set(atomics_tests
    # --- Simple non-concurrent tests
    nonconcurrent_store_load_islockfree
    nonconcurrent_increment
    nonconcurrent_compare_exchange
    # nonconcurrent_wait_timeout

    # --- Concurrent tests without synchronization
    concurrent_store_load
    concurrent_increment
    concurrent_compare_exchange

    # --- Non-concurrent wait/notify tests
    nonconcurrent_notify_zero
    nonconcurrent_wait_not_equal
    nonconcurrent_wait_i32_not_equal
    nonconcurrent_wait_i64_not_equal
    nonconcurrent_waitasync_store_notify

    # --- Concurrent wait/notify tests
    concurrent_wait_store_notify
    concurrent_countdownlatch
    # rendezvous_channel
    # cyclic_barrier
)

set(tests_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(tests_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

set(coroutine_options --coroutine-impl=threaded --coroutine-js-mode=false --coroutine-workers-count=10 --gc-type=g1-gc)

add_custom_target(ets_test_suite_atomics)

# This is a 'run_int_jit_aot_ets_code' function but without JIT
function(run_atomics_tests ETS_SRC WORK_DIR TARGET)
    set(oneValueArgs OPT_LEVEL)
    set(noValues SKIP_ARM32_COMPILER)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "${noValues}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_custom_target(${TARGET})

    # TODO(egor-porsev): Add jit and aot. Currently, they do not work because of the data race issue #13551
    # int
    run_int_ets_code(${ETS_SRC} ${WORK_DIR} ${TARGET}-ets-int OPT_LEVEL ${ARG_OPT_LEVEL} RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS})
    if(TARGET ${TARGET}-ets-int)
        add_dependencies(${TARGET} ${TARGET}-ets-int)
    endif()
endfunction()

foreach(test ${atomics_tests})
    set(test_in "${tests_in_dir}/${test}.sts")
    set(test_out_dir "${tests_out_dir}/${test}")
    file(MAKE_DIRECTORY ${test_out_dir})
    set(target ets_test_suite_atomics_${test})

    run_atomics_tests(${test_in} ${test_out_dir} ${target} OPT_LEVEL 0 RUNTIME_EXTRA_OPTIONS ${coroutine_options})

    if(TARGET ${target})
        add_dependencies(ets_test_suite_atomics ${target})
    endif()
endforeach()
