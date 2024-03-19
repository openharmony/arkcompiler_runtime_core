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

add_custom_target(ets_checked_tests COMMENT "Running ets core tests with checkers")
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
    if (${TEST_TYPE} MATCHES "ets")
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

    if (${TEST_TYPE} MATCHES "ets")
        set(COMMAND_TOKEN "//!")
    else()
        set(COMMAND_TOKEN "#!")
    endif()
    set(OPTIONS "--load-runtimes=ets" "--run-gc-in-place" "--boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc")
    set(ES2PANDA_OPTIONS --thread=0 --extension=ets)

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
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/string_test.ets SUPPORT_RELEASE true)
    if (PANDA_TARGET_AMD64 OR PANDA_TARGET_ARM64)
        # inlining of the following intrinsics is only enabled for these two targets
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_isFinite.ets)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_isNaN.ets)
        # CallLaunch isn't supported for arm32 (need to fix storing args in stack)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/launch_test.ets SUPPORT_RELEASE true)
        # AOT mode for arm32 is not supported
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_equals.ets SUPPORT_RELEASE true)
        # LoadAndInitClass in AOT mode for arm32 is not supported
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/optimize_load_readonly_static.ets)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/optimize_load_readonly_instance.ets)
    endif()
    if (PANDA_TARGET_ARM64)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/memory_coalescing.ets)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/charAt.ets)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/memory_coalescing1.ets)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/memory_coalescing2.ets)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/memory_coalescing3.ets)
        panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/memory_coalescing4.ets)
    endif()
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/gotos.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_runtime_equals.ets SUPPORT_RELEASE true)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_abs.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_double_compare.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_equals.ets)
    # NOTE(vpukhov): #16340 requires ets.equals profiling
    # panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_inline_intrinsics.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_min.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_max.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_mod.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_round.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_floor.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_trunc.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_ceil.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/checks_elimination_osr.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/cleanup_call_inlined.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_isinteger.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/hoist_loop_inv_bounds_check.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_length.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_isempty.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_substring.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_builder_reserve.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_concat.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_string_concat_loop.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/multiarray.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_char_isuppercase.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_lse_phi_test.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch1.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch2.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch3.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch4.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch5.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch6.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch7.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_try_catch8.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_static_lookup.pa)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_nullish.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_nested_loops.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_stringbuilder.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_stringbuilder_oom.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/bounds_analysis_overflow.ets)
    panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ets_const_array.ets)
endif()

panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/load_array.ets)
panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/optimize_negation.ets)
