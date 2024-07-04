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

panda_ets_interop_js_test(ets_interop_js_test_settimeout
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.sts
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testSetTimeout"
)

panda_ets_interop_js_test(ets_interop_js_test_cleartimeout
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.sts
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testClearTimeout"
)

panda_ets_interop_js_test(ets_interop_js_test_setinterval
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.sts
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
    LAUNCHER_ARGS "testSetInterval"
)
