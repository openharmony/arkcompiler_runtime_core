# Copyright (c)Huawei Device Co., Ltd.
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

if (PANDA_TARGET_ARM32)
    return()
endif()

set(gc_test_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(gc_test_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(ets_test_suite_gc)

function(add_ets_gc_test)
    set(prefix ARG)
    set(singleValues FILE MODE)
    set(multiValues OPTIONS)
    cmake_parse_arguments(${prefix} "" "${singleValues}" "${multiValues}" ${ARGN})

    get_filename_component(TEST_NAME "${ARG_FILE}" NAME_WE)

    set(test_in "${gc_test_in_dir}/${ARG_FILE}")
    set(test_out_dir "${gc_test_out_dir}/${TEST_NAME}")
    string(TOLOWER "${ARG_MODE}" run_mode)
    set(target ets_test_suite_gc_${TEST_NAME}-ets-${run_mode})

    if ("${ARG_MODE}" STREQUAL "INT")
        run_int_ets_code(${test_in} ${test_out_dir} ${target} RUNTIME_EXTRA_OPTIONS ${ARG_OPTIONS})
    elseif ("${ARG_MODE}" STREQUAL "AOT")
        run_aot_ets_code(aot ${test_in} ${test_out_dir} ${target} RUNTIME_EXTRA_OPTIONS ${ARG_OPTIONS})
    elseif ("${ARG_MODE}" STREQUAL "JIT")
        run_jit_ets_code(${test_in} ${test_out_dir} ${target} RUNTIME_EXTRA_OPTIONS ${ARG_OPTIONS})
    else()
        message(FATAL_ERROR "Mode ${ARG_MODE} is not defined. Available modes: INT/AOT/JIT")
    endif()
    add_dependencies(ets_test_suite_gc ${target})
endfunction()

function(add_ets_napi_gc_test)
    set(prefix ARG)
    set(singleValues FILE CPP_SOURCE)
    set(multiValues OPTIONS)
    cmake_parse_arguments(${prefix} "" "${singleValues}" "${multiValues}" ${ARGN})

    get_filename_component(TEST_NAME "${ARG_FILE}" NAME_WE)
    set(test_in "${gc_test_in_dir}/${ARG_FILE}")
    set(test_out_abc "${gc_test_out_dir}/${TEST_NAME}.abc")
    set(target ets_test_suite_gc_${TEST_NAME})

    compile_ets_code(${test_in} ${test_out_abc} ${target})

    panda_ets_add_gtest(
        NO_CORES
        NAME ${target}_cpp_test
        SOURCES
            ${ARG_CPP_SOURCE}
        LIBRARIES
            arkbase arkfile arkruntime arkassembler
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
        PANDA_STD_LIB
            ${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
        DEPS_TARGETS
            etsstdlib
            ${target}
    )
    add_dependencies(ets_test_suite_gc ${target}_cpp_test_gtests)
endfunction()

# mm_tests target will launch tests that use GC the most
add_dependencies(mm_tests
                ets_test_suite_coroutines
                ets_test_suite_gc
                ets_test_suite_strings
                bouncing_pandas_ets-ets-int
                bouncing_pandas_ets-ets-int-cpp
                bouncing_pandas_ets-ets-jit
                bouncing_peas_ets-ets-int
                bouncing_peas_ets-ets-int-cpp
                bouncing_peas_ets-ets-jit
                etsnapi_test_suite)
if(NOT CMAKE_CROSSCOMPILING)
    add_dependencies(mm_tests
                    bouncing_pandas_ets-ets-aot
                    bouncing_peas_ets-ets-aot)
endif()

add_ets_gc_test(FILE pin_object.sts OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(FILE get_object_address_test.sts OPTIONS "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(FILE space_type_test.sts OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(FILE weak_ref_test.sts OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(FILE concurrent_start_gc.sts OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(FILE register_native_allocation_test.sts OPTIONS "--gc-type=g1-gc" "--run-gc-in-place" "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(FILE finalization_registry_test.sts OPTIONS "--gc-type=g1-gc" "--run-gc-in-place" "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(FILE scheduled_gc.sts OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug-never" "--gc-use-nth-alloc-trigger=true" MODE "INT")
add_ets_gc_test(FILE scheduled_gc_trigger_exception.sts OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug-never" MODE "INT")

add_ets_gc_test(FILE postpone_gc_test.sts OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug" MODE "INT")

add_ets_gc_test(FILE run_gc_every_safepoint.sts OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug" "--run-gc-in-place" "--run-gc-every-safepoint" MODE "INT")
add_ets_gc_test(FILE run_gc_every_safepoint.sts OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug" "--run-gc-in-place" "--run-gc-every-safepoint" MODE "JIT")
if (NOT CMAKE_CROSSCOMPILING)
    add_ets_gc_test(FILE run_gc_every_safepoint.sts OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug" "--run-gc-in-place" "--run-gc-every-safepoint" MODE "AOT")
endif()
add_ets_gc_test(FILE heap_info_test.sts OPTIONS MODE "INT")
add_ets_gc_test(FILE free_heap_size_test.sts OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug" "--run-gc-in-place" "--gc-workers-count=1" MODE "INT")
# Running tests via API calls
add_ets_napi_gc_test(FILE pending_exception_gc_test.sts CPP_SOURCE pending_exception_test.cpp OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug-never")
