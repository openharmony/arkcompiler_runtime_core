# Copyright (c) 2026 Huawei Device Co., Ltd.
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

set(suite_target_name ets_debug_session_attach_test_suite)
add_custom_target(${suite_target_name})

# Build native library
set(ani_lib_name ets_debug_session_attach_lib)
panda_add_library(${ani_lib_name} SHARED utils.cpp)
panda_target_include_directories(${ani_lib_name} PRIVATE
    ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani
    ${PANDA_ROOT}
    ${PANDA_BINARY_ROOT}
)
panda_target_link_libraries(${ani_lib_name}
    ani_interface
    arkruntime
)
panda_add_sanitizers(
    TARGET ${ani_lib_name}
    SANITIZERS ${PANDA_SANITIZERS_LIST}
)

set(ani_lib_dir ${CMAKE_CURRENT_BINARY_DIR}/lib)
file(MAKE_DIRECTORY ${ani_lib_dir})
set_target_properties(${ani_lib_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${ani_lib_dir})

set(
    PANDA_RUN_PREFIX
    LD_LIBRARY_PATH=${ani_lib_dir}
    ${PANDA_RUN_PREFIX}
)

set(additional_dependencies_target_name ets_debug_session_attach_tests_deps)
add_custom_target(${additional_dependencies_target_name} DEPENDS ${ani_lib_name})

# Add dependency on ANI library build
function(add_target_with_dependencies test_suite target additional_dependencies)
    add_dependencies(${test_suite} ${target})
    if (additional_dependencies)
        add_dependencies(${target} ${additional_dependencies})
    endif()
endfunction()

function(add_debug_session_attach_test_impl test_target_name test_name test_work_dir test_ets_source runtime_extra_options fe_options)
    set(noValues IS_STACKLESS)
    cmake_parse_arguments(ARG "${noValues}" "" "" ${ARGN})

    run_int_ets_code(${test_target_name}-ets-int ${test_work_dir}
        SOURCES "${test_ets_source}"
        RUNTIME_EXTRA_OPTIONS ${runtime_extra_options}
        COMPILER_EXTRA_OPTIONS ${fe_options}
    )

    run_aot_ets_code(aot ${test_target_name}-ets-aot ${test_work_dir}
        SOURCES ${test_ets_source}
        ENTRYPOINT ${test_name}
        RUNTIME_EXTRA_OPTIONS ${runtime_extra_options}
        COMPILER_EXTRA_OPTIONS ${fe_options}
    )

    # NOTE(dslynko): stackless is not supported yet in LLVM AOT
    if (NOT ARG_IS_STACKLESS AND PANDA_LLVM_AOT)
        run_aot_ets_code(llvm ${test_target_name}-ets-llvmaot ${test_work_dir}
            SOURCES ${test_ets_source}
            ENTRYPOINT ${test_name}
            RUNTIME_EXTRA_OPTIONS ${runtime_extra_options}
            COMPILER_EXTRA_OPTIONS ${fe_options}
        )
    endif()

    get_ets_int_targets_list(int_targets_list ${test_target_name}-ets-int)
    foreach(int_target ${int_targets_list})
        add_target_with_dependencies(${test_target_name} ${int_target} ${additional_dependencies_target_name})
    endforeach()

    if (NOT PANDA_TARGET_ARM32 AND NOT CMAKE_CROSSCOMPILING)
        add_target_with_dependencies(${test_target_name} ${test_target_name}-ets-aot ${additional_dependencies_target_name})
        if (NOT ARG_IS_STACKLESS AND PANDA_LLVM_AOT)
            add_target_with_dependencies(${test_target_name} ${test_target_name}-ets-llvmaot
                ${additional_dependencies_target_name}
            )
        endif()
    endif()
endfunction()

# Adds test with target name `ets_debug_session_attach_${file_name}`
function(add_debug_session_attach_test ets_file)
    set(noValues USES_CONCURRENCY SUPPORTS_STACKLESS)
    cmake_parse_arguments(ARG "${noValues}" "" "" ${ARGN})

    get_filename_component(test_name ${ets_file} NAME_WE)
    set(
        test_ets_source
        "${CMAKE_CURRENT_SOURCE_DIR}/${ets_file}"
        "${CMAKE_CURRENT_SOURCE_DIR}/utils.ets"
    )

    set(impls "STACKFUL")
    get_target_property(STACKLESS_STDLIB etsstdlib.stackless FILE)
    if (ARG_SUPPORTS_STACKLESS)
        set(impls "STACKFUL" "STACKLESS")
    endif()

    foreach(impl ${impls})
        string(TOLOWER "${impl}" impl_name)
        set(test_target_name ets_debug_session_attach_${test_name}_${impl_name})
        add_custom_target(${test_target_name} DEPENDS ${additional_dependencies_target_name})

        set(runtime_extra_options
            --debugger-attach-allowed=true
            --compiler-enable-jit=false
            "--coroutine-impl=${impl_name}"
        )
        set(fe_options "")
        set(is_stackless "")

        if (impl STREQUAL "STACKLESS")
            list(APPEND fe_options "--stackless-coros=true")
            set(is_stackless "IS_STACKLESS")
            set(BOOT_PANDA_FILES_OLD ${BOOT_PANDA_FILES})
            set(BOOT_PANDA_FILES ${STACKLESS_STDLIB})
        elseif(CMAKE_BUILD_TYPE STREQUAL "Debug" OR
               PANDA_ENABLE_ADDRESS_SANITIZER OR
               PANDA_ENABLE_THREAD_SANITIZER)
            list(APPEND runtime_extra_options "--coroutine-stack-size-pages=128")
        endif()

        set(test_work_dir ${CMAKE_CURRENT_BINARY_DIR}/${test_name}_${impl_name})

        add_debug_session_attach_test_impl(
            ${test_target_name}
            ${test_name}
            ${test_work_dir}
            "${test_ets_source}"
            "${runtime_extra_options}"
            "${fe_options}"
            "${is_stackless}"
        )

        if (impl STREQUAL "STACKLESS")
            set(BOOT_PANDA_FILES ${BOOT_PANDA_FILES_OLD})
        endif()

        add_dependencies(${suite_target_name} ${test_target_name})
    endforeach()
endfunction()

add_debug_session_attach_test(test_attach_main_and_throw.ets)
add_debug_session_attach_test(test_attach_main_double_attach.ets)
add_debug_session_attach_test(test_attach_main_loop.ets)
add_debug_session_attach_test(test_attach_main_native_throw.ets)
add_debug_session_attach_test(test_attach_main_prelinked_call.ets)
add_debug_session_attach_test(test_attach_main_proxy.ets)
add_debug_session_attach_test(test_attach_main_return.ets)
add_debug_session_attach_test(test_attach_main_via_ani.ets)
add_debug_session_attach_test(test_attach_main_via_reflection.ets)
# NOTE(dslynko): enable stackless tests for the rest of tests after runtime fully supports stackless
add_debug_session_attach_test(test_attach_from_worker.ets USES_CONCURRENCY)
add_debug_session_attach_test(test_attach_main_with_workers.ets USES_CONCURRENCY)
add_debug_session_attach_test(test_attach_from_launch.ets USES_CONCURRENCY)
add_debug_session_attach_test(test_attach_main_with_launch.ets USES_CONCURRENCY SUPPORTS_STACKLESS)
