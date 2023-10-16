# Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

# TODO(konstanting, #I67QXC): test both threaded and stackful implementations where possible
set(coroutine_tests
    launch_yield
    await
    launch_exception
    launch_instruction
    launch_return
    async_call
    launch_instr_array
)
if (NOT PANDA_TARGET_ARM32)
    list(APPEND coroutine_tests launch_oom)
    list(APPEND coroutine_tests multiple_launch)
    list(APPEND coroutine_tests launch_n_workers)
    list(APPEND coroutine_tests class_load_race)
endif()

set(coroutine_tests_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(coroutine_tests_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

if (PANDA_TARGET_ARM32)
    set(stackful_impl_option "--coroutine-impl=threaded")
    set(threaded_impl_option "--coroutine-impl=threaded")
    set(js_mode_option "--coroutine-js-mode=false")
else()
    set(stackful_impl_option "--coroutine-impl=stackful")
    set(threaded_impl_option "--coroutine-impl=threaded")
    set(js_mode_option "--coroutine-js-mode=true")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR PANDA_ENABLE_ADDRESS_SANITIZER OR PANDA_ENABLE_THREAD_SANITIZER)
        list(APPEND stackful_impl_option "--coroutine-stack-size-pages=128")
    endif()
endif()

add_custom_target(ets_test_suite_coroutines)

foreach(test ${coroutine_tests})
    set(test_in "${coroutine_tests_in_dir}/${test}.ets")
    set(test_out_dir "${coroutine_tests_out_dir}/${test}")
    file(MAKE_DIRECTORY ${test_out_dir})
    set(target ets_test_suite_coroutines_${test})

    if (test STREQUAL "launch_n_workers")
        run_int_jit_aot_ets_code(${test_in} ${test_out_dir} ${target} SKIP_ARM32_COMPILER RUNTIME_EXTRA_OPTIONS "--coroutine-workers-count=0" ${stackful_impl_option})
    elseif(test STREQUAL "class_load_race")
        run_int_jit_aot_ets_code(${test_in} ${test_out_dir} ${target} SKIP_ARM32_COMPILER RUNTIME_EXTRA_OPTIONS "--coroutine-workers-count=0" ${stackful_impl_option})
    elseif (test STREQUAL "async_call")
        run_int_jit_aot_ets_code(${test_in} ${test_out_dir} ${target} SKIP_ARM32_COMPILER RUNTIME_EXTRA_OPTIONS ${js_mode_option} ${stackful_impl_option})
    elseif (test STREQUAL "launch_oom")
        # AddressSanitizer: stack-buffer-overflow #12611
        if (NOT PANDA_ENABLE_ADDRESS_SANITIZER)
            run_int_jit_aot_ets_code(${test_in} ${test_out_dir} ${target} SKIP_ARM32_COMPILER SKIP_OSR RUNTIME_EXTRA_OPTIONS "--gc-type=epsilon" ${stackful_impl_option} "--compiler-enable-osr=false")
        endif()
    elseif (test STREQUAL "launch_exception")
        # llvmaot handles the recursive stack eater function incorrectly
        set(PANDA_LLVMAOT_TEMP ${PANDA_LLVMAOT})
        set(PANDA_LLVMAOT 0)
        run_int_jit_aot_ets_code(${test_in} ${test_out_dir} ${target} SKIP_ARM32_COMPILER RUNTIME_EXTRA_OPTIONS ${stackful_impl_option})
        set(PANDA_LLVMAOT ${PANDA_LLVMAOT_TEMP})
    elseif (test STREQUAL "launch_return")
        # Issue 12886
        run_int_ets_code(${test_in} ${test_out_dir} ${target} RUNTIME_EXTRA_OPTIONS ${stackful_impl_option})
    elseif (test STREQUAL "multiple_launch")
        # Depends on issue 12886
        run_int_ets_code(${test_in} ${test_out_dir} ${target} RUNTIME_EXTRA_OPTIONS "--use-coroutine-pool=true" ${stackful_impl_option})
    else()
        run_int_jit_aot_ets_code(${test_in} ${test_out_dir} ${target} SKIP_ARM32_COMPILER RUNTIME_EXTRA_OPTIONS ${stackful_impl_option})
    endif()

    if(TARGET ${target})
        add_dependencies(ets_test_suite_coroutines ${target})
    endif()
endforeach()
