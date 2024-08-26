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

add_custom_target(ets_test_suite_coroutines)

# Add ets coroutine tests targets
#
# Example usage:
#
# add_ets_coroutines_test(FILE ets_file
#                         [SKIP_ARM32_COMPILER]
#                         [OPTIONS "--gc-type=epsilon"]
#                         IMPL "THREADED" "STACKFUL"
#                         OPTION_SETS_THREADED "DEFAULT"
#                         OPTION_SETS_STACKFUL "DEFAULT" "JS" "POOL" "JS_POOL"
#                         WORKERS "AUTO" "ONE"
#                         MODE "INT" "JIT" "AOT" "LLVMAOT" "JITOSR"
# )
# This funciont will create a number of tests as a cartesian product of IMPL, OPTION_SETS_${IMPL}, WORKERS, and MODES, e.g.
#                         THREADED DEFAULT AUTO INT
#                         THREADED DEFAULT AUTO JIT
#                         ...
#                         STACKFUL JS_POOL ONE JITOSR
#
# SKIP_ARM32_COMPILER and OPTIONS will apply to all tests
function(add_ets_coroutines_test)
    set(prefix ARG)
    set(noValues SKIP_ARM32_COMPILER)
    set(singleValues FILE)
    set(multiValues OPTIONS IMPL OPTION_SETS_THREADED OPTION_SETS_STACKFUL WORKERS MODE)
    cmake_parse_arguments(${prefix} "${noValues}" "${singleValues}" "${multiValues}" ${ARGN})

    if (ARG_SKIP_ARM32_COMPILER)
        set(SKIP_ARM32_COMPILER "SKIP_ARM32_COMPILER")
    endif()

    get_filename_component(test_name "${ARG_FILE}" NAME_WE)

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
                if(option_set STREQUAL "JS")
                    set(additional_options "--coroutine-js-mode=true")
                elseif(option_set STREQUAL "POOL")
                    set(additional_options "--use-coroutine-pool=true")
                elseif(option_set STREQUAL "JS_POOL")
                    set(additional_options "--coroutine-js-mode=true" "--use-coroutine-pool=true")
                elseif(option_set STREQUAL "STATS")
                    set(additional_options "--coroutine-dump-stats=true")
                endif()
                string(TOLOWER "${option_set}" options_name)

                add_ets_test(ets_test_suite_coroutines
                             FILE ${ARG_FILE}
                             ${SKIP_ARM32_COMPILER}
                             TEST_NAME "${test_name}_${impl_name}_${options_name}_workers_${workers_count}"
                             OPTIONS ${ARG_OPTIONS} ${impl_option} ${additional_options} ${workers_option}
                             MODE ${ARG_MODE}
                )
            endforeach()
        endforeach()
    endforeach()
endfunction()

add_ets_coroutines_test(FILE async_call.sts
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "JS"
                        WORKERS "ONE"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE async_call.sts
                        SKIP_ARM32_COMPILER
                        IMPL "THREADED" "STACKFUL"
                        OPTION_SETS_THREADED "DEFAULT"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "ONE"
                        MODE "INT" "JIT"
)

add_ets_coroutines_test(FILE launch_instruction.sts
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "JS_POOL"
                        WORKERS "ONE"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE launch_instruction.sts
                        SKIP_ARM32_COMPILER
                        IMPL "THREADED" "STACKFUL"
                        OPTION_SETS_THREADED "DEFAULT"
                        OPTION_SETS_STACKFUL "DEFAULT" "JS" "POOL"
                        WORKERS "ONE"
                        MODE "INT" "JIT"
)

add_ets_coroutines_test(FILE await.sts
                        SKIP_ARM32_COMPILER
                        IMPL "THREADED"
                        OPTION_SETS_THREADED "DEFAULT"
                        WORKERS "AUTO" "ONE"
                        MODE "INT" "JIT"
)

add_ets_coroutines_test(FILE launch_launch.sts
                        SKIP_ARM32_COMPILER
                        IMPL "THREADED"
                        OPTION_SETS_THREADED "DEFAULT"
                        WORKERS "AUTO" "ONE"
                        MODE "INT" "JIT"
)

if (NOT PANDA_TARGET_ARM32)
    add_ets_coroutines_test(FILE await.sts
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "JS_POOL"
                            WORKERS "AUTO"
                            MODE "INT" "JIT" "AOT"
    )

    add_ets_coroutines_test(FILE await.sts
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT" "JS" "POOL" "JS_POOL"
                            WORKERS "ONE"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE launch_exception.sts
                            SKIP_ARM32_COMPILER
                            IMPL "THREADED" "STACKFUL"
                            OPTION_SETS_THREADED "DEFAULT"
                            OPTION_SETS_STACKFUL "DEFAULT" "JS" "POOL" "JS_POOL"
                            WORKERS "AUTO" "ONE"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE launch_instr_array.sts
                            SKIP_ARM32_COMPILER
                            IMPL "THREADED" "STACKFUL"
                            OPTION_SETS_THREADED "DEFAULT"
                            OPTION_SETS_STACKFUL "DEFAULT" "JS" "POOL" "JS_POOL"
                            WORKERS "AUTO"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE launch_instr_array.sts
                            SKIP_ARM32_COMPILER
                            IMPL "THREADED" "STACKFUL"
                            OPTION_SETS_THREADED "DEFAULT"
                            OPTION_SETS_STACKFUL "DEFAULT" "JS" "JS_POOL"
                            WORKERS "ONE"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE launch_instr_array.sts
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "POOL"
                            WORKERS "ONE"
                            MODE "INT" "JIT" "AOT"
    )

    add_ets_coroutines_test(FILE class_load_race.sts
                            SKIP_ARM32_COMPILER
                            IMPL "THREADED" "STACKFUL"
                            OPTION_SETS_THREADED "DEFAULT"
                            OPTION_SETS_STACKFUL "DEFAULT" "JS" "POOL" "JS_POOL"
                            WORKERS "AUTO" "ONE"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE launch_n_workers.sts
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT"
                            WORKERS "AUTO"
                            MODE "INT" "JIT" "AOT"
    )

    add_ets_coroutines_test(FILE launch_n_workers.sts
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "JS" "POOL" "JS_POOL"
                            WORKERS "AUTO"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE launch_oom.sts
                            SKIP_ARM32_COMPILER
                            OPTIONS "--gc-type=epsilon"
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "JS_POOL"
                            WORKERS "ONE"
                            MODE "INT" "JIT" "AOT"
    )

    add_ets_coroutines_test(FILE launch_oom.sts
                            SKIP_ARM32_COMPILER
                            OPTIONS "--gc-type=epsilon"
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT" "JS" "POOL"
                            WORKERS "ONE"
                            MODE "INT" "JIT"
    )

    # Depends on issue 12886
    add_ets_coroutines_test(FILE multiple_launch.sts
                            SKIP_ARM32_COMPILER
                            IMPL "THREADED" "STACKFUL"
                            OPTION_SETS_THREADED "DEFAULT"
                            OPTION_SETS_STACKFUL "DEFAULT" "JS" "JS_POOL"
                            WORKERS "AUTO"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE multiple_launch.sts
                            SKIP_ARM32_COMPILER
                            IMPL "THREADED" "STACKFUL"
                            OPTION_SETS_THREADED "DEFAULT"
                            OPTION_SETS_STACKFUL "JS" "POOL"
                            WORKERS "ONE"
                            MODE "INT" "JIT"
    )

    add_ets_coroutines_test(FILE multiple_launch.sts
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "POOL"
                            WORKERS "AUTO"
                            MODE "INT" "JIT" "AOT"
    )

    add_ets_coroutines_test(FILE multiple_launch.sts
                            SKIP_ARM32_COMPILER
                            IMPL "STACKFUL"
                            OPTION_SETS_STACKFUL "DEFAULT" "JS_POOL"
                            WORKERS "ONE"
                            MODE "INT" "JIT" "AOT"
    )

    add_ets_coroutines_test(FILE concurrent_await.sts
                        SKIP_ARM32_COMPILER
                        IMPL "THREADED" "STACKFUL"
                        OPTION_SETS_THREADED "DEFAULT"
                        WORKERS "AUTO" "ONE"
                        MODE "INT" "JIT"
    )
endif()
# # Issue 12886
add_ets_coroutines_test(FILE launch_return.sts
                        SKIP_ARM32_COMPILER
                        IMPL "THREADED" "STACKFUL"
                        OPTION_SETS_THREADED "DEFAULT"
                        OPTION_SETS_STACKFUL "DEFAULT" "JS"
                        WORKERS "ONE"
                        MODE "INT" "JIT"
)

add_ets_coroutines_test(FILE launch_yield.sts
                        SKIP_ARM32_COMPILER
                        IMPL "THREADED" "STACKFUL"
                        OPTION_SETS_THREADED "DEFAULT"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "ONE"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE launch_yield.sts
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "JS" "JS_POOL"
                        WORKERS "ONE"
                        MODE "INT" "JIT"
)

add_ets_coroutines_test(FILE affinity.sts
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT"
)

add_ets_coroutines_test(FILE wake_for_another_event.sts
                        SKIP_ARM32_COMPILER
                        IMPL "THREADED" "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE await_standalone_promise.sts
                        SKIP_ARM32_COMPILER
                        IMPL "THREADED" "STACKFUL"
                        OPTION_SETS_STACKFUL "DEFAULT"
                        WORKERS "AUTO"
                        MODE "INT" "JIT" "AOT"
)

add_ets_coroutines_test(FILE stats.sts
                        SKIP_ARM32_COMPILER
                        IMPL "STACKFUL"
                        OPTION_SETS_STACKFUL "STATS"
                        WORKERS "AUTO"
                        MODE "INT"
)

