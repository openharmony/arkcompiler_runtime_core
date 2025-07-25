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

panda_ets_interop_js_test(ets_interop_js_test_settimeout
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.ets
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testSetTimeout"
)

panda_ets_interop_js_test(ets_interop_js_test_cleartimeout
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.ets
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testClearTimeout"
)

panda_ets_interop_js_test(ets_interop_js_test_setinterval
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.ets
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testSetInterval"
)

panda_ets_interop_js_test(ets_interop_js_test_settimeout_execute_order
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.ets
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testSetTimeoutExecuteOrder"
)

panda_ets_interop_js_test(ets_interop_js_test_settimeout_invoke_callback_with_specified_parameter
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.ets
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testSetTimeoutInvokeCallbackWithSpecifiedParameter"
)

panda_ets_interop_js_test(ets_interop_js_test_settimeout_invoke_callback_with_4_specified_parameters
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.ets
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testSetTimeoutInvokeCallbackWith4SpecifiedParameters"
)

panda_ets_interop_js_test(ets_interop_js_test_setinterval_invoke_callback_with_specified_parameter
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.ets
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testSetIntervalInvokeCallbacWithSpecifiedParameter"
)

panda_ets_interop_js_test(ets_interop_js_test_setinterval_invoke_callback_with_4_specified_parameters
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.ets
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testSetIntervalInvokeCallbacWith4SpecifiedParameters"
)

panda_ets_interop_js_test(ets_interop_js_test_settimeout_with_negitive_delay
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.ets
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testSetTimeoutWithNegitiveDelay"
)

panda_ets_interop_js_test(ets_interop_js_test_setinterval_with_negitive_delay
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.ets
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testSetIntervalWithNegitiveDelay"
)

panda_ets_interop_js_test(ets_interop_js_test_settimeout_with_string
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.ets
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testSetTimeoutWithString"
)

panda_ets_interop_js_test(ets_interop_js_test_setinterval_with_string
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.ets
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testSetIntervalWithString"
)

panda_ets_interop_js_test(ets_interop_js_test_clear_timer_cross_worker
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.ets
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testClearTimerCrossWorker"
)
