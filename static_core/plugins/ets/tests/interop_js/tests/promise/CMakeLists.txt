# Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})
# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise1
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testPendingPromise"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise2
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testResolvedPromise"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise3
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testRejectedPromise"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise4
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testThenBeforeResolve"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise5
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testPromiseEmptyThen"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise6
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testPromiseChain"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_catch_finally_chain
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testPromiseCatchFinallyChain"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_resolve_promise_then_finally
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testResolvePromiseThenFinally"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_reject_promise_then_finally
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testRejectPromiseThenFinally"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise7
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_promise_to_js_test.js
#     LAUNCHER_ARGS "testReturnPromise" "Panda"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise8
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_promise_to_js_test.js
#     LAUNCHER_ARGS ".testReturnPromiseFromAsync" "Panda"
# )

# NOTE: #23066
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise9
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_pending_promise_to_js_test.js
#     LAUNCHER_ARGS "then"
# )

# NOTE: #23066
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise10
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_pending_promise_to_js_test.js
#     LAUNCHER_ARGS "await"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise11
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/await_js_promise.js
#     LAUNCHER_ARGS "pending"
#     COMPILATION_JS_WITH_CJS_ON
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise12
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/await_js_promise.js
#     LAUNCHER_ARGS "resolved"
#     COMPILATION_JS_WITH_CJS_ON
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_await_promise
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS ".testAwaitPromise"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_async_bool
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_promise_to_js_test.js
#     LAUNCHER_ARGS "testAsyncBoolean" true 2
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_async_byte
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_promise_to_js_test.js
#     LAUNCHER_ARGS ".testAsyncByte" 127 2
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_async_char
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_promise_to_js_test.js
#     LAUNCHER_ARGS ".testAsyncChar" "x" 2
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_async_short
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_promise_to_js_test.js
#     LAUNCHER_ARGS "testAsyncShort" 32767 2
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_async_int
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_promise_to_js_test.js
#     LAUNCHER_ARGS ".testAsyncInt" 5 2
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_async_long
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_promise_to_js_test.js
#     LAUNCHER_ARGS ".testAsyncLong" 9223372036854775807 2
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_async_float
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_promise_to_js_test.js
#     LAUNCHER_ARGS ".testAsyncFloat" 3.14 2
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_async_double
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_promise_to_js_test.js
#     LAUNCHER_ARGS "testAsyncDouble" 3.2 2
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_async_string
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_promise_to_js_test.js
#     LAUNCHER_ARGS ".testAsyncString" "Panda" 2
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_async_void1
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_promise_to_js_test.js
#     LAUNCHER_ARGS ".testAsyncVoid" "undefined" 2
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_async_void2
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_promise_to_js_test.js
#     LAUNCHER_ARGS ".testAsyncVoidEmpty" "undefined" 2
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_async_void3
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/return_promise_to_js_test.js
#     LAUNCHER_ARGS "testAsyncVoidNothing" "undefined" 2
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_all_empty
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS ".testPromiseAllEmpty"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_all_resolved
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testPromiseAllResolved"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_all_rejected
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testPromiseAllRejected"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_all_raw
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testPromiseAllRawValues"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_all_iterable
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testPromiseAllIterable"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_allsettled_empty
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testPromiseAllSettledEmpty"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_allsettled
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testPromiseAllSettled"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_any_empty
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testPromiseAnyEmpty"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_any_resolved
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testPromiseAnyResolved"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_any_rejected
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testPromiseAnyRejected"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_any_raw
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS ".testPromiseAnyRawValues"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_any_iterable
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testPromiseAnyRejected"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_race_resolved
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS ".testPromiseRaceResolved"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_race_rejected
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
#     LAUNCHER_ARGS "testPromiseRaceRejected"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_to_js_and_back
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_to_js_and_back_test.js
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_to_ets_and_back
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_to_ets_and_back_test.js
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_resolve_js_promise_from_non_main_coro
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/resolve_js_promise_from_non_main_coro_test.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/resolve_js_promise_from_non_main_coro_test.js
#     LAUNCHER_ARGS ".testPromiseResolve"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_schedule_main_coro
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/schedule_main_coro.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/schedule_main_coro.js
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_proxy
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test.js
#     JS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test_exports.js
#     ETS_CONFIG ${ETS_CONFIG}
#     LAUNCHER_ARGS ".testProxyPromise"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_proxy_with_await
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test.js
#     JS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test_exports.js
#     ETS_CONFIG ${ETS_CONFIG}
#     LAUNCHER_ARGS ".testAwaitPromiseProxy"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_proxy_reject
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test.js
#     JS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test_exports.js
#     ETS_CONFIG ${ETS_CONFIG}
#     LAUNCHER_ARGS ".testProxyPromiseReject"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_proxy_with_await_reject
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test.js
#     JS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test_exports.js
#     ETS_CONFIG ${ETS_CONFIG}
#     LAUNCHER_ARGS ".testAwaitPromiseProxyReject"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_proxy_with_await_reject_fail
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test.js
#     JS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test_exports.js
#     ETS_CONFIG ${ETS_CONFIG}
#     LAUNCHER_ARGS ".testAwaitPromiseProxyRejectShouldFail"
# )

# NOTE: #23100
# panda_ets_interop_js_test_arkjsvm(ets_interop_js_test_promise_proxy_with_then_await_reject_fail
#     ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test.ets
#     JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test.js
#     JS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_proxy_test_exports.js
#     ETS_CONFIG ${ETS_CONFIG}
#     LAUNCHER_ARGS ".testAwaitPromiseProxyThenRejectShouldFail"
# )

panda_ets_interop_js_test(ets_interop_js_test_promise_then_on_rejection
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/promise_tests.ets
    JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_promise_test.js
    LAUNCHER_ARGS "testPromiseThenOnRejection"
)
