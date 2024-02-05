# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

set(WARMUP 1000)
set(ITERS 10000)

set(HEAVY_RUN OFF)
if (HEAVY_RUN)
    set(WARMUP 100000)
    set(ITERS 1000000)
endif()

list(APPEND PERF_TESTS
    "void_call"
    "void_call_10str"
    "void_call_10dbl"
    "str_call"
    "anystr_call"
    "dbl_call"
    "anydbl_call"
    "anyobj_call"
)

add_custom_target(ets_interop_js_perf_tests COMMENT "Running all js interop perf tests")

foreach(TEST ${PERF_TESTS})
    panda_ets_interop_js_test(ets_interop_js_perf_test_${TEST}
        ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/test.ets
        JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_test.js
        LAUNCHER_ARGS ${TEST} ${WARMUP} ${ITERS}
    )
    add_dependencies(ets_interop_js_perf_tests ets_interop_js_perf_test_${TEST})

    panda_ets_interop_js_test(ets_interop_js_perf_test_frontend_${TEST}
        ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/test_frontend.ets
        JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_test.js
        LAUNCHER_ARGS ${TEST} ${WARMUP} ${ITERS}
        ETS_CONFIG "${PANDA_BINARY_ROOT}/tests/ets_interop_js/arktsconfig.json"
        ETS_VERIFICATOR_ERRORS "ForLoopCorrectlyInitializedForAll"
    )
    add_dependencies(ets_interop_js_perf_tests ets_interop_js_perf_test_frontend_${TEST})
endforeach()
