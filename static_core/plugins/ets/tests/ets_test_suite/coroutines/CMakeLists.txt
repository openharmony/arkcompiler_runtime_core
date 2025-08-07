# Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#                         IMPL "THREADED" "STACKFUL"
#                         OPTION_SETS_THREADED "DEFAULT"
#                         OPTION_SETS_STACKFUL "DEFAULT" "POOL" "MIGRATION_AWAIT"
#                         WORKERS "AUTO" "ONE"
#                         MODE "INT" "JIT" "AOT" "LLVMAOT" "JITOSR"
# )
# This funciont will create a number of tests as a cartesian product of IMPL, OPTION_SETS_${IMPL}, WORKERS, and MODES, e.g.
#                         THREADED DEFAULT AUTO INT
#                         THREADED DEFAULT AUTO JIT
#                         ...
#                         STACKFUL ONE JITOSR
#
# SKIP_ARM32_COMPILER and OPTIONS will apply to all tests
function(add_ets_coroutines_test)
    set(prefix ARG)
    set(noValues SKIP_ARM32_COMPILER LIST_UNHANDLED)
    set(singleValues FILE)
    set(multiValues OPTIONS IMPL OPTION_SETS_THREADED OPTION_SETS_STACKFUL WORKERS MODE)
    cmake_parse_arguments(${prefix} "${noValues}" "${singleValues}" "${multiValues}" ${ARGN})

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

        foreach(workers_count ${ARG_WORKERS})
            set(workers_option "--coroutine-workers-count=0")
            if (workers_count STREQUAL "ONE")
                set(workers_option "--coroutine-workers-count=1")
            endif()
            string(TOLOWER "${workers_count}" workers_count)

            foreach(option_set ${ARG_OPTION_SETS_${impl}})
                set(additional_options "")
                if(option_set STREQUAL "POOL")
                    set(additional_options "--use-coroutine-pool=true")
                elseif(option_set STREQUAL "STATS")
                    set(additional_options "--coroutine-dump-stats=true")
                elseif(option_set STREQUAL "MIGRATION_AWAIT")
                    set(additional_options "--coroutine-enable-features:migration,migrate-await")
                endif()
                string(TOLOWER "${option_set}" options_name)

                add_ets_test(ets_test_suite_coroutines
                             FILE ${ARG_FILE}
                             ${SKIP_ARM32_COMPILER}
                             TEST_NAME "${test_name}_${impl_name}_${options_name}_workers_${workers_count}"
                             OPTIONS ${ARG_OPTIONS} ${impl_option} ${additional_options} ${workers_option} ${list_unhandled_option}
                             MODE ${ARG_MODE}
                )
            endforeach()
        endforeach()
    endforeach()
endfunction()

add_ets_coroutines_test(FILE allhandled_rejection.ets
                        SKIP_ARM32_COMPILER
                        LIST_UNHANDLED
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO" "ONE"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE unhandled_rejection.ets
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

add_ets_coroutines_test(FILE launch_instruction.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "POOL"
                        WORKERS "ONE"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE launch_instruction.ets
                        SKIP_ARM32_COMPILER
                        IMPL "THREADED" "STACKFUL"
                        OPTION_SETS_THREADED "DEFAULT"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "ONE"
                        MODE "INT" "JIT"
)

add_ets_coroutines_test(FILE await.ets
                        SKIP_ARM32_COMPILER
                        IMPL "THREADED"
                        OPTION_SETS_THREADED "DEFAULT"
                        WORKERS "AUTO" "ONE"
                        MODE "INT" "JIT"
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
                            OPTION_SETS_STACKFUL "POOL"
                            WORKERS "AUTO"
                            MODE "INT" "JIT" "AOT"
    )

    add_ets_coroutines_test(FILE await.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT" "POOL"
                            WORKERS "ONE"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE launch_exception.ets
                            SKIP_ARM32_COMPILER
                            IMPL "THREADED" "STACKFUL"
                            OPTION_SETS_THREADED "DEFAULT"
                            OPTION_SETS_STACKFUL "DEFAULT" "POOL"
                            WORKERS "AUTO" "ONE"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE launch_instr_array.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL" "THREADED"
                            OPTION_SETS_THREADED "DEFAULT"
                            OPTION_SETS_STACKFUL "DEFAULT" "POOL"
                            WORKERS "AUTO" "ONE"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE launch_instr_array.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "POOL"
                            WORKERS "ONE"
                            MODE "AOT"
    )

    # NOTE(26403) Profilesaver is turned off because of data race
    add_ets_coroutines_test(FILE class_load_race.ets
                            SKIP_ARM32_COMPILER
                            OPTIONS "--profilesaver-enabled=false"
                            IMPL "THREADED" "STACKFUL"
                            OPTION_SETS_THREADED "DEFAULT"
                            OPTION_SETS_STACKFUL "DEFAULT" "POOL"
                            WORKERS "AUTO" "ONE"
                            MODE "INT" "JIT"
    )

    # NOTE(26403) Profilesaver is turned off because of data race
    add_ets_coroutines_test(FILE launch_n_workers.ets
                            SKIP_ARM32_COMPILER
                            OPTIONS "--profilesaver-enabled=false"
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT"
                            WORKERS "AUTO"
                            MODE "INT" "JIT" "AOT"
    )

    # NOTE(26403) Profilesaver is turned off because of data race
    add_ets_coroutines_test(FILE launch_n_workers.ets
                            SKIP_ARM32_COMPILER
                            OPTIONS "--profilesaver-enabled=false"
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "POOL"
                            WORKERS "AUTO"
                            MODE "INT" "JIT"
    )

    # NOTE(26403) Profilesaver is turned off because of data race
    add_ets_coroutines_test(FILE launch_oom.ets
                            SKIP_ARM32_COMPILER
                            OPTIONS "--gc-type=epsilon" "--profilesaver-enabled=false"
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "POOL"
                            WORKERS "ONE"
                            MODE "INT" "JIT" "AOT"
    )

    # NOTE(26403) Profilesaver is turned off because of data race
    add_ets_coroutines_test(FILE launch_oom.ets
                            SKIP_ARM32_COMPILER
                            OPTIONS "--gc-type=epsilon" "--profilesaver-enabled=false"
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT"
                            WORKERS "ONE"
                            MODE "INT" "JIT"
    )

    # Depends on issue 12886
    add_ets_coroutines_test(FILE multiple_launch.ets
                            SKIP_ARM32_COMPILER
                            IMPL "THREADED" "STACKFUL"
                            OPTION_SETS_THREADED "DEFAULT"
                            OPTION_SETS_STACKFUL "DEFAULT" "POOL"
                            WORKERS "AUTO" "ONE"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE multiple_launch.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "POOL"
                            WORKERS "AUTO"
                            MODE "AOT"
    )

    add_ets_coroutines_test(FILE multiple_launch.ets
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT" "POOL"
                            WORKERS "ONE"
                            MODE "AOT"
    )

    add_ets_coroutines_test(FILE concurrent_await.ets
                        SKIP_ARM32_COMPILER
                        IMPL "THREADED" "STACKFUL"
                        OPTION_SETS_THREADED "DEFAULT"
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
                        OPTION_SETS_THREADED "DEFAULT"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "ONE"
                        MODE "INT"
)

add_ets_coroutines_test(FILE launch_yield.ets
                        SKIP_ARM32_COMPILER
                        IMPL "THREADED" "STACKFUL"
                        OPTION_SETS_THREADED "DEFAULT"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "ONE"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE launch_yield.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "POOL"
                        WORKERS "ONE"
                        MODE "INT" "JIT"
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
                        IMPL "THREADED" "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE await_standalone_promise.ets
                        SKIP_ARM32_COMPILER
                        IMPL "THREADED" "STACKFUL"
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
                        IMPL "THREADED" "STACKFUL"
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

add_ets_coroutines_test(FILE await_migrate.ets
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "MIGRATION_AWAIT"
                        WORKERS "AUTO"
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
                        IMPL "THREADED" "STACKFUL"
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
