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
    runtime_linker
)

set(linker_tests_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(linker_tests_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(ets_test_suite_linker)

function(add_target_with_dependencies test_suite target additional_dependencies)
    add_dependencies(${test_suite} ${target})
    if (additional_dependencies)
        add_dependencies(${target} ${additional_dependencies})
    endif()
endfunction()

function(add_linker_test test additional_dependencies)
    set(noValues SKIP_ARM32_COMPILER)
    cmake_parse_arguments(ARG "${noValues}" "" "" ${ARGN})

    set(should_skip_compiler "")
    if (ARG_SKIP_ARM32_COMPILER)
        set(should_skip_compiler "SKIP_ARM32_COMPILER")
    endif()

    set(test_out_dir "${linker_tests_out_dir}/${test}")
    set(test_in "${linker_tests_in_dir}/${test}.sts")
    set(target ets_test_suite_linker_${test})

    run_int_jit_aot_ets_code(${test_in} ${test_out_dir} ${target} ${should_skip_compiler})

    # int
    get_ets_int_targets_list(int_targets_list ${target}-ets-int)
    foreach(int_target ${int_targets_list})
        add_target_with_dependencies(ets_test_suite_linker ${int_target} "${additional_dependencies}")
    endforeach()

    if (ARG_SKIP_ARM32_COMPILER AND PANDA_TARGET_ARM32)
        return()
    endif()

    # jit
    add_target_with_dependencies(ets_test_suite_linker ${target}-ets-jit "${additional_dependencies}")

    if (NOT CMAKE_CROSSCOMPILING)
        # aot
        add_target_with_dependencies(ets_test_suite_linker ${target}-ets-aot "${additional_dependencies}")

        if (PANDA_LLVM_AOT)
            # llvm-aot
            add_target_with_dependencies(ets_test_suite_linker ${target}-ets-llvmaot "${additional_dependencies}")
        endif()
    endif()

    if (PANDA_TARGET_ARM64)
        # jit-osr
        add_target_with_dependencies(ets_test_suite_linker ${target}-ets-jit-osr "${additional_dependencies}")
    endif()

endfunction()

foreach(test ${linker_tests})
    add_linker_test(${test} "")
endforeach()

function(test_runtime_linker_extensions test)
    file(GLOB abc_runtime_linker_sources "${linker_tests_in_dir}/abc_runtime_linker_sources/*.sts")
    set(additional_abc_dir "${linker_tests_out_dir}/abc_runtime_linker_sources")
    set(additional_abc_files_list "")
    set(compilations_targets_list "")

    foreach(ets_source ${abc_runtime_linker_sources})
        get_filename_component(src_filename ${ets_source} NAME)
        set(output_abc ${additional_abc_dir}/${src_filename}.abc)
        list(APPEND additional_abc_files_list ${output_abc})

        set(compilation_target ${test}-${src_filename}-ets-es2panda)
        list(APPEND compilations_targets_list ${compilation_target})
        compile_ets_code(${ets_source} ${output_abc} ${compilation_target}
            OPT_LEVEL ${ARG_OPT_LEVEL}
            COMPILER_EXTRA_OPTIONS "--ets-module")
    endforeach()

    list(JOIN additional_abc_files_list ":" additional_abc_files)
    set(PANDA_RUN_PREFIX ADDITIONAL_ABC_FILES=${additional_abc_files} ${PANDA_RUN_PREFIX})

    add_custom_target(additional_dependencies DEPENDS ${compilations_targets_list})
    add_linker_test(${test} additional_dependencies SKIP_ARM32_COMPILER)

endfunction()

test_runtime_linker_extensions(runtime_linker_extensions)

function(test_app_linker_context_exception TARGET WORK_DIR)
    # Must provide a valid abc file, so that runtime would start creating application linker context
    set(EXISTING_SOURCE "${linker_tests_in_dir}/abc_runtime_linker_sources/additional1.sts")
    set(EXISTING_ABC "${WORK_DIR}/${TARGET}-existing.abc")
    set(ABC_TARGET ${TARGET}-abc)
    compile_ets_code(${EXISTING_SOURCE} ${EXISTING_ABC} ${ABC_TARGET})

    # Provide non-existing abc file on which application linker context will fail
    set(NON_EXISTING_ABC_FILE "${WORK_DIR}/${TARGET}-non-existing.abc")

    add_custom_target(${TARGET}
        COMMAND ${linker_tests_in_dir}/test_app_linker_context_exception.sh
            --run-prefix="${PANDA_RUN_PREFIX}"
            --ark-binary=$<TARGET_FILE:ark>
            --boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
            --non-existing-abc=${NON_EXISTING_ABC_FILE}
            --entry-abc=${EXISTING_ABC}
            --entrypoint=additional1.ETSGLOBAL::main
        DEPENDS ark etsstdlib ${ABC_TARGET}
    )
endfunction()

test_app_linker_context_exception(ets_app_linker_context_tests ${linker_tests_out_dir})
add_dependencies(ets_tests ets_app_linker_context_tests)
