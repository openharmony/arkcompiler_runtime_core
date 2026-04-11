# Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

add_custom_target(ets_test_suite_coroutines)

# Add ets coroutine tests targets
#
# Example usage:
#
# add_ets_coroutines_test(FILE ets_file
#                         [SKIP_ARM32_COMPILER]
#                         [LIST_UNHANDLED]
#                         [OPTIONS "--gc-type=epsilon"]
#                         IMPL "STACKFUL"
#                         OPTION_SETS_STACKFUL "DEFAULT" "POOL" "MIGRATE_AWAKENED" "MIGRATION"
#                         WORKERS "AUTO" "ONE" "THREE"
#                         MODE "INT" "JIT" "AOT" "LLVMAOT" "JITOSR"
# )
# This funcion will create a number of tests as a cartesian product of IMPL, OPTION_SETS_${IMPL}, WORKERS, and MODES, e.g.
#                         STACKFUL DEFAULT AUTO INT
#                         STACKFUL DEFAULT AUTO JIT
#                         ...
#                         STACKFUL ONE JITOSR
#
# SKIP_ARM32_COMPILER and OPTIONS will apply to all tests
function(add_ets_coroutines_test)
    set(prefix ARG)
    set(noValues SKIP_ARM32_COMPILER LIST_UNHANDLED)
    set(singleValues FILE)
    set(multiValues OPTIONS IMPL OPTION_SETS_STACKFUL OPTION_SETS_STACKLESS WORKERS MODE)
    cmake_parse_arguments(${prefix} "${noValues}" "${singleValues}" "${multiValues}" ${ARGN})

    # NOTE(26403) Profilesaver is turned off beause of integer overflow
    list(APPEND ARG_OPTIONS "--profilesaver-enabled=false" "--incremental-profilesaver-enabled=false")

    if (ARG_SKIP_ARM32_COMPILER)
        set(SKIP_ARM32_COMPILER "SKIP_ARM32_COMPILER")
    endif()

    get_filename_component(test_name "${ARG_FILE}" NAME_WE)

    if (ARG_LIST_UNHANDLED)
        set(list_unhandled_option "--list-unhandled-on-exit:jobs,promises")
    endif()

    foreach(impl ${ARG_IMPL})
        string(TOLOWER "${impl}" impl_name)
        set(impl_option "--coroutine-impl=${impl_name}")
        if(impl STREQUAL "STACKFUL" AND
           (CMAKE_BUILD_TYPE STREQUAL "Debug" OR PANDA_ENABLE_ADDRESS_SANITIZER OR PANDA_ENABLE_THREAD_SANITIZER))
            list(APPEND impl_option "--coroutine-stack-size-pages=128")
        endif()
        set(fe_options "")
        if (impl STREQUAL "STACKLESS")
            list(APPEND fe_options "--stackless-coros=true")
            list(APPEND impl_option "--interpreter-type=cpp")
        endif()

        foreach(workers_count ${ARG_WORKERS})
            set(workers_option "--coroutine-workers-count=0")
            if (workers_count STREQUAL "ONE")
                set(workers_option "--coroutine-workers-count=1")
            elseif (workers_count STREQUAL "THREE")
                set(workers_option "--coroutine-workers-count=3")
            endif()
            string(TOLOWER "${workers_count}" workers_count)

            foreach(option_set ${ARG_OPTION_SETS_${impl}})
                set(additional_options "")
                if(option_set STREQUAL "NO_POOL")
                    set(additional_options "--use-coroutine-pool=false")
                elseif(option_set STREQUAL "STATS")
                    set(additional_options "--coroutine-dump-stats=true")
                elseif(option_set STREQUAL "MIGRATE_AWAKENED")
                    set(additional_options "--coroutine-enable-features:migrate-awakened")
                elseif(option_set STREQUAL "MIGRATION")
                    set(additional_options "--coroutine-enable-features:migration")
                endif()
                string(TOLOWER "${option_set}" options_name)

                add_ets_test(ets_test_suite_coroutines
                             FILE ${ARG_FILE}
                             ${SKIP_ARM32_COMPILER}
                             TEST_NAME "${test_name}_${impl_name}_${options_name}_workers_${workers_count}"
                             OPTIONS ${ARG_OPTIONS} ${impl_option} ${additional_options} ${workers_option} ${list_unhandled_option}
                             MODE ${ARG_MODE}
                             COMPILER_EXTRA_OPTIONS ${fe_options}
                )
            endforeach()
        endforeach()
    endforeach()
endfunction()

# NOTE(@srokashevich, #29320): enable with tsan after fix
if (NOT PANDA_ENABLE_THREAD_SANITIZER)
add_ets_coroutines_test(FILE allhandled_rejection.ets
                        SKIP_ARM32_COMPILER
                        LIST_UNHANDLED
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO" "ONE"
                        MODE "INT" "JIT" "AOT"
)
endif()

add_ets_coroutines_test(FILE unhandled_rejection.ets
                        SKIP_ARM32_COMPILER
                        LIST_UNHANDLED
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO" "ONE"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE unhandled_rejection_event.ets
                        SKIP_ARM32_COMPILER
                        LIST_UNHANDLED
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO" "ONE"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE unhandled_rejection_promise.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO" "ONE"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE dump_unhandled_jobs.ets
                        SKIP_ARM32_COMPILER
                        LIST_UNHANDLED
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO" "ONE"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE async_call.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "ONE"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE global_timer_in_static_init.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "ONE"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE launch_instruction.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "NO_POOL"
                        WORKERS "ONE"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE launch_instruction.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "ONE"
                        MODE "INT" "JIT"
)

add_ets_coroutines_test(FILE launch_instruction.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKLESS"
                        OPTION_SETS_STACKLESS "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE launch_launch.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL" # Issue 24058
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO" "ONE"
                        MODE "INT" "JIT"
)

if (NOT PANDA_TARGET_ARM32)
    add_ets_coroutines_test(FILE await.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "NO_POOL"
                            WORKERS "AUTO"
                            MODE "INT" "JIT" "AOT"
    )

    add_ets_coroutines_test(FILE await.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT" "NO_POOL"
                            WORKERS "ONE"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE launch_exception.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT" "NO_POOL"
                            WORKERS "AUTO" "ONE"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE launch_instr_array.ets
                            SKIP_ARM32_COMPILER
                            OPTION_SETS_STACKFUL "DEFAULT" "NO_POOL"
                            WORKERS "AUTO" "ONE"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE launch_instr_array.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "NO_POOL"
                            WORKERS "ONE"
                            MODE "AOT"
    )

    add_ets_coroutines_test(FILE launch_instr_array.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKLESS"
                            OPTION_SETS_STACKLESS "DEFAULT"
                            WORKERS "AUTO"
                            MODE "INT"
    )

    add_ets_coroutines_test(FILE class_load_race.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT" "POOL"
                            WORKERS "AUTO" "ONE"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE launch_n_workers.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT"
                            WORKERS "AUTO"
                            MODE "INT" "JIT" "AOT"
    )

    add_ets_coroutines_test(FILE launch_n_workers.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "POOL"
                            WORKERS "AUTO"
                            MODE "INT" "JIT"
    )


    # limit with 10'000 coroutine can be faced only on 64bit arch
    if (NOT PANDA_ENABLE_THREAD_SANITIZER AND NOT PANDA_ENABLE_ADDRESS_SANITIZER)
        add_ets_coroutines_test(FILE launch_7000.ets
                                SKIP_ARM32_COMPILER
                                LIST_UNHANDLED
                                IMPL "STACKFUL"
                                OPTION_SETS_STACKFUL "DEFAULT"
                                WORKERS "AUTO" "ONE"
                                MODE "INT" "JIT" "AOT"
        )

        add_ets_coroutines_test(FILE coroutine_limits.ets
                                SKIP_ARM32_COMPILER
                                IMPL "STACKFUL"
                                OPTION_SETS_STACKFUL "DEFAULT"
                                WORKERS "AUTO"
                                MODE "INT"
        )

      add_ets_coroutines_test(FILE coroutine_counter.ets
                                SKIP_ARM32_COMPILER
                                IMPL "STACKFUL"
                                OPTION_SETS_STACKFUL "DEFAULT"
                                WORKERS "AUTO"
                                MODE "INT"
        )
    endif()

    # NOTE(30627) Temporarily disabled due failures under TSan
    if (NOT PANDA_ENABLE_THREAD_SANITIZER)
        add_ets_coroutines_test(FILE launch_oom.ets
                                SKIP_ARM32_COMPILER
                                OPTIONS "--gc-type=epsilon" "--coroutines-stack-mem-limit=268435456"
                                IMPL "STACKFUL"
                                OPTION_SETS_STACKFUL "POOL"
                                WORKERS "ONE"
                                MODE "INT" "JIT" "AOT"
        )
    endif()

    add_ets_coroutines_test(FILE launch_oom.ets
                            SKIP_ARM32_COMPILER
                            OPTIONS "--gc-type=epsilon" "--coroutines-stack-mem-limit=268435456"
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT"
                            WORKERS "ONE"
                            MODE "INT" "JIT"
    )

    # Depends on issue 12886
    add_ets_coroutines_test(FILE multiple_launch.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT" "NO_POOL"
                            WORKERS "AUTO" "ONE"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE multiple_launch.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "NO_POOL"
                            WORKERS "AUTO"
                            MODE "AOT"
    )

    add_ets_coroutines_test(FILE multiple_launch.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT" "NO_POOL"
                            WORKERS "ONE"
                            MODE "AOT"
    )

    add_ets_coroutines_test(FILE concurrent_await.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        WORKERS "AUTO" "ONE"
                        MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE eworker_unlimited.ets
                            SKIP_ARM32_COMPILER
                            OPTIONS "--coroutine-e-workers-limit=100500"
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT"
                            WORKERS "ONE"
                            MODE "INT"
    )
endif()

add_ets_coroutines_test(FILE eworker_id.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

# # Issue 12886
add_ets_coroutines_test(FILE launch_return.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL" # Issue 24058
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "ONE"
                        MODE "INT"
)

add_ets_coroutines_test(FILE launch_return.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKLESS"
                        OPTION_SETS_STACKLESS "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE launch_yield.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "ONE"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE launch_yield.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "NO_POOL"
                        WORKERS "ONE"
                        MODE "INT" "JIT"
)

add_ets_coroutines_test(FILE launch_yield.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKLESS"
                        OPTION_SETS_STACKLESS "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE affinity.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE wake_for_another_event.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE wake_for_another_event.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKLESS"
                        OPTION_SETS_STACKLESS "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE await_standalone_promise.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE stats.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "STATS"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE eaworker_coroutines.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE eworkers_reached_limit.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE eworker_interop_exception.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE launch.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE setTimeout.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE scale_workers_pool.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "ONE"
                        MODE "INT"
)

# NOTE(alimovilya, #33501) enable after fix
# add_ets_coroutines_test(FILE migrate_awakened_coro.ets
#                         SKIP_ARM32_COMPILER
#                         IMPL "STACKFUL"
#                         OPTION_SETS_STACKFUL "MIGRATE_AWAKENED"
#                         WORKERS "THREE"
#                         MODE "INT"
# )

add_ets_coroutines_test(FILE migration.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "MIGRATION"
                        WORKERS "THREE"
                        MODE "INT"
)

add_ets_coroutines_test(FILE job_test.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE execution_order.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "ONE"
                        MODE "INT"
)

add_ets_coroutines_test(FILE eworker_parameter.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE await_async_return_reject_promise.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE eworker_post_to_main.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE worker_group.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE sync_primitives.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE legacy_atomics_wait_notify.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "THREE"
                        MODE "INT" "JIT" "AOT"
)

# Stackless async coroutine tests (.pa files)
# NOTE: skip arm32 because of launch instruction support
if (NOT PANDA_TARGET_ARM32)
    set(STACKLESS_ASYNC_TESTS
        stackless_async
        # stackless_async_params
    )

    set(COMMON_RUNTIME_OPTIONS "--coroutine-impl=stackless" "--coroutine-workers-count=0" "--verification-mode=disabled")
    set(COMMONT_COMPILER_OPTIONS "--compiler-inlining=false")

    foreach(test_name ${STACKLESS_ASYNC_TESTS})
        # llvm currently is not supported
        foreach(interpreter_type "cpp" "irtoc")
            panda_add_asm_file(
                FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/ets_test_suite/coroutines/${test_name}.pa
                TARGET ets_coroutines_${test_name}_${interpreter_type}
                DEPENDS etsstdlib
                LANGUAGE_CONTEXT ets
                ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
                RUNTIME_OPTIONS ${COMMON_RUNTIME_OPTIONS} "--interpreter-type=${interpreter_type}"
            )
            add_dependencies(ets_test_suite_coroutines ets_coroutines_${test_name}_${interpreter_type})
            add_dependencies(ets_run_int_ets_code_tests ets_coroutines_${test_name}_${interpreter_type})

        endforeach()

        # AOT mode
        # no PANDA_TARGET_ARM64 currently
        if(PANDA_COMPILER_ENABLE AND PANDA_TARGET_AMD64)
            panda_add_asm_file(
                FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/ets_test_suite/coroutines/${test_name}.pa
                TARGET ets_coroutines_${test_name}_aot
                DEPENDS etsstdlib
                LANGUAGE_CONTEXT ets
                AOT_MODE TRUE
                ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
                RUNTIME_OPTIONS ${COMMON_RUNTIME_OPTIONS} "--compiler-enable-jit=false"
                COMPILER_OPTIONS ${COMMONT_COMPILER_OPTIONS}
            )
            add_dependencies(ets_test_suite_coroutines ets_coroutines_${test_name}_aot)
            add_dependencies(ets_run_aot_ets_code_tests ets_coroutines_${test_name}_aot)

            if (OFF) # PANDA_LLVM_AOT is not supported for stackless
                panda_add_asm_file(
                    FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/ets_test_suite/coroutines/${test_name}.pa
                    TARGET ets_coroutines_${test_name}_llvmaot
                    DEPENDS etsstdlib
                    LANGUAGE_CONTEXT ets
                    AOT_MODE TRUE
                    AOT_COMPILER llvm
                    ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
                    RUNTIME_OPTIONS ${COMMON_RUNTIME_OPTIONS} "--compiler-enable-jit=false"
                    COMPILER_OPTIONS ${COMMONT_COMPILER_OPTIONS}
                )
                add_dependencies(ets_test_suite_coroutines ets_coroutines_${test_name}_llvmaot)
                add_dependencies(ets_run_aot_ets_code_tests ets_coroutines_${test_name}_llvmaot)
            endif()
        endif()
    endforeach()
endif()

add_subdirectory(dfx)
add_subdirectory(no_suitable_worker)
