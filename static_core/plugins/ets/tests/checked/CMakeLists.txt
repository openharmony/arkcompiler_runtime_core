# Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

add_custom_target(ets_checked_tests COMMENT "Running sts core tests with checkers")
add_dependencies(ets_checked_tests ark)

function(panda_add_checked_test_ets)
    set(prefix ARG)
    set(singleValues NAME FILE SUPPORT_RELEASE LIBCORE)
    set(multiValues AUX_FILES EXT_FILES)
    cmake_parse_arguments(${prefix} "" "${singleValues}" "${multiValues}" ${ARGN})

    # Events does not work in Release mode
    if (NOT DEFINED ARG_SUPPORT_RELEASE AND "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        return()
    endif()

    if(NOT PANDA_COMPILER_ENABLE)
        return()
    endif()

    if ("${ARG_NAME}" STREQUAL "")
        get_filename_component(TEST_NAME "${ARG_FILE}" NAME_WE)
        set(TEST_NAME "${TEST_NAME}.checked")
    else()
        set(TEST_NAME "${ARG_NAME}.checked")
    endif()
    set(CHECKER "${PANDA_ROOT}/tests/checked/checker.rb")
    set(TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}")
    set(BINARY_FILE "${TEST_DIR}/test.abc")
    set(BUILD_LOG   "${TEST_DIR}/build.log")

    if (NOT DEFINED ARG_FILE)
        message(FATAL_ERROR "Mandatory FILE argument is not defined.")
    endif()

    file(MAKE_DIRECTORY "${TEST_DIR}")
    set(ABC_TARGET ${TEST_NAME}-ets-es2panda)
    get_filename_component(TEST_TYPE "${ARG_FILE}" EXT)
    if (${TEST_TYPE} MATCHES "sts")
        compile_ets_code(${ARG_FILE} ${BINARY_FILE} ${ABC_TARGET})
    else()
    # Compile assembly file
        add_custom_command(OUTPUT "${BINARY_FILE}"
                COMMENT "Building ${TEST_NAME}"
                COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark_asm> --log-file ${BUILD_LOG} ${ARG_FILE} ${BINARY_FILE}
                DEPENDS ${assembler} "${ARG_FILE}"
                WORKING_DIRECTORY "${ARG_WORKING_DIR}")
    endif()

    set(EXT_FILES "")
    foreach(APP_FILE ${ARG_EXT_FILES})
        get_filename_component(APP_FILE_BASE "${APP_FILE}" NAME_WE)
        set(APP_FILE_OUTPUT "${TEST_DIR}/${APP_FILE_BASE}.abc")
        set(ABC_APP_TARGET ${TEST_NAME}-${APP_FILE_BASE}-ets-es2panda)
        compile_ets_code(${APP_FILE} ${APP_FILE_OUTPUT} ${ABC_APP_TARGET})
        # TODO(pishin): pass external files via --app-panda-files (now it doesn't work)
        list(APPEND stdlibs "${APP_FILE_OUTPUT}")
        list(APPEND EXT_FILES "${APP_FILE_OUTPUT}")
    endforeach()

    if (${TEST_TYPE} MATCHES "sts")
        set(COMMAND_TOKEN "//!")
    else()
        set(COMMAND_TOKEN "#!")
    endif()
    set(OPTIONS "--load-runtimes=ets" "--run-gc-in-place" "--coroutine-workers-count=1" "--boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc")
    set(ES2PANDA_OPTIONS --thread=0 --extension=sts)
    if (NOT PANDA_CI_TESTING_MODE STREQUAL "Nightly")
        set (OPTIONS ${OPTIONS} "--compiler-check-final=true")
    endif()

    if (PANDA_LLVM_AOT)
        set(WITH_LLVM "--with-llvm")
    endif()

    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        set(RELEASE_OPT "--release")
    endif()

    if (PANDA_TARGET_AMD64)
        set(ARCHITECTURE "x64")
    elseif (PANDA_TARGET_ARM64)
        set(ARCHITECTURE "arm64")
    else()
        set(ARCHITECTURE "arm32")
    endif()

    # Run checker
    add_custom_target(${TEST_NAME}
                      COMMAND ${CHECKER} --source ${ARG_FILE}
                                         --panda $<TARGET_FILE:ark>
                                         --paoc $<TARGET_FILE:ark_aot>
                                         --frontend ${es2panda_bin}
                                         --run-prefix \"${PANDA_RUN_PREFIX}\"
                                         --test-file ${BINARY_FILE}
                                         --panda-options \"${OPTIONS}\"
                                         --paoc-options \"${OPTIONS}\"
                                         --frontend-options \"${ES2PANDA_OPTIONS}\"
                                         --command-token \"${COMMAND_TOKEN}\"
                                         --arch ${ARCHITECTURE}
                                         ${RELEASE_OPT}
                                         ${WITH_LLVM}
                      WORKING_DIRECTORY ${TEST_DIR}
                      COMMENT "Running ${TEST_NAME} checked test"
                      DEPENDS ${BINARY_FILE} ${EXT_FILES})

    add_dependencies(ets_checked_tests ${TEST_NAME})

    foreach(AUX_FILE ${ARG_AUX_FILES})
        get_filename_component(AUX_NAME "${AUX_FILE}" NAME_WE)
        set(AUX_NAME "${AUX_NAME}.checked")
        add_dependencies(${TEST_NAME} ${AUX_NAME})
    endforeach()

endfunction()

# Disable checked tests with sanitizers for arm64, since it takes too long time running.
if (PANDA_TARGET_AMD64 OR NOT PANDA_ARM64_TESTS_WITH_SANITIZER)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/string_test.sts SUPPORT_RELEASE true)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/string_split_test.sts)
    if (PANDA_TARGET_AMD64 OR PANDA_TARGET_ARM64)
        # inlining of the following intrinsics is only enabled for these two targets
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_isFinite.sts)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_isNaN.sts)
        # CallLaunch isn't supported for arm32 (need to fix storing args in stack)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/launch_test.sts SUPPORT_RELEASE true)
        # AOT mode for arm32 is not supported
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_equals.sts SUPPORT_RELEASE true)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/inlining_test/inlining_order_1.sts)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/inlining_test/inlining_order_2.sts)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/inlining_test/inlining_order_3.sts)
        # LoadAndInitClass in AOT mode for arm32 is not supported
        # NOTE(pantos) Issue #17103 reassign readonly property
        # panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/optimize_load_readonly_static.sts)
        # panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/optimize_load_readonly_instance.sts)
    endif()
    if (PANDA_TARGET_ARM64)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/memory_coalescing.sts)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/charAt.sts)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/memory_coalescing1.sts)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/memory_coalescing2.sts)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/memory_coalescing3.sts)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/memory_coalescing4.sts)
    endif()
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/gotos.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_runtime_equals.sts SUPPORT_RELEASE true)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_abs.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_double_compare.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_equals.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_escape_array.sts)
    # NOTE(vpukhov): #16340 requires ets.equals profiling
    # panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_inline_intrinsics.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_min.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_max.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_mod.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_round.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_floor.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_trunc.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_ceil.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/checks_elimination_osr.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/cleanup_call_inlined.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_isinteger.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/hoist_loop_inv_bounds_check.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_length.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_isempty.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_substring.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_append_merge_part1.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_append_merge_part2.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_export.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_part1.sts AUX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_export.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_part2.sts AUX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_export.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_part3.sts AUX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_export.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_part4.sts AUX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_export.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_part5.sts AUX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_export.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_part6.sts AUX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_export.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_part7.sts AUX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_export.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_part8.sts AUX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_merge_uber_export.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_reserve_uber.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_reserve_part01-10.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_reserve_part11-19.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_reserve_part20-29.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_reserve_part30-44.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_concat.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_concat_func.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_concat_loop.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/multiarray.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_char_isuppercase.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_lse_phi_test.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch1.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch2.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch3.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch4.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch5.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch6.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch7.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch8.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch9.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch10.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch11.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch12.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch13.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch14.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch15.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_static_lookup.pa)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_static_lookup_8bit.pa)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_static_lookup_16bit.pa)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_nullish.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_nested_loops.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_stringbuilder.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_stringbuilder_oom.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/bounds_analysis_overflow.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/bounds_analysis_phi_in_loops.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_const_array.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/hoist_checkcast.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/escape_deoptimization.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/fast_divisor.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_pea_loads_upward_propagation.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_sum_elem_array.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_safepoints.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_overflow.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_catch_assert.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_catch_assert1.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_catch_assert2.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_catch_assert3.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_typeof_compare.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_typeof_compare1.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_typeof_compare2.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_typeof_compare3.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_typeof_compare4.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_typeof_compare5.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_typeof_compare6.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_async_lock_callback.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_async_lock_callback1.sts)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_promise_launch.sts)
endif()

panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/escape_analysis_casted.sts)
panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/load_array.sts)
panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/optimize_negation.sts)
panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/llvm_infer_flags.sts)
