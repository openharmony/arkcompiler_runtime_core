# Copyright (c) 2024 Huawei Device Co., Ltd.
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

add_custom_target(ets_test_suite_containers)

# Add ets containers tests targets
#
# Example usage:
#
# add_ets_containers_test(FILE ets_file
#                         [SKIP_ARM32_COMPILER]
#                         [OPTIONS "--gc-type=epsilon"]
#                         IMPL "THREADED" "STACKFUL"
#                         OPTION_SETS_THREADED "DEFAULT"
#                         OPTION_SETS_STACKFUL "DEFAULT" "JS" "POOL" "JS_POOL"
#                         WORKERS "AUTO" "ONE"
#                         MODE "INT" "JIT" "AOT" "LLVMAOT" "JITOSR"
# )
# SKIP_ARM32_COMPILER and OPTIONS will apply to all tests
function(add_ets_containers_test)
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
                endif()
                string(TOLOWER "${option_set}" options_name)

                add_ets_test(ets_test_suite_containers
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

add_ets_containers_test(FILE blocking_queue.sts
                        SKIP_ARM32_COMPILER
                        OPTIONS "--gc-type=g1-gc" "--run-gc-in-place" "--gc-trigger-type=debug-never" "--compiler-ignore-failures=true"
                        IMPL "THREADED" "STACKFUL"
                        OPTION_SETS_THREADED "DEFAULT"
                        OPTION_SETS_STACKFUL "DEFAULT" "JS" "POOL"
                        WORKERS "ONE" "AUTO"
                        MODE "INT" "JIT" "AOT"
)
