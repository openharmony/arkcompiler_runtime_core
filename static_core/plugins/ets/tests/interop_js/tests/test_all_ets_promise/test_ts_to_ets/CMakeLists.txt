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

set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

set(TEST_FUNCTIONS
    checkPromiseNumber
    checkPromiseString
    checkPromiseBoolean
    checkPromiseBigInt
    checkPromiseNull
    checkPromiseUndefined
    checkPromiseNan
    checkPromiseInfinity
    checkThenTwoParamsFulfilled
    checkThenTwoParamsRejected
    checkThenOnlyOnFulfilled
    checkThenPromiseUndefined
    checkThenNoParamsRejected
    checkCatchStandardRejected
    checkCatchFulfilledPassthrough
    checkCatchReturnPromise
    checkCatchThrowChain
    checkFinallyFulfilled
    checkFinallyRejectedFromTS
    checkFinallyNoParam
    checkFinallyThrowOverridesFulfilled
    checkFinallyThrowOverridesRejected
    checkFinallyReturnValueIgnored
    checkResolvePromiseResolve
    checkResolvePromiseReject
    checkAllFulfilled
    checkAllRejected
    checkStaticAllEmpty
    checkStaticAllWithPromises
    checkStaticAllWithRejected
    checkStaticAllMixed
    checkStaticAllWithString
    checkStaticAnyEmpty
    checkAnyFulfilled
    checkStaticAnyAllRejected
    checkStaticAnyMixed
    checkStaticAnyFirstWins
    checkRaceFulfilled
    checkRaceRejected
    checkStaticRaceEmpty
    checkStaticRacePlainValues
    checkStaticRaceFulfilledWins
    checkStaticRaceRejectedWins
    checkStaticRaceMixedImmediateWins
    checkStaticRaceRejectImmediate
    checkStaticRaceWithString
    checkcbLambdaNoWait
    checkcbLambda
    checkInstanceMethodNoWait
    checkInstanceMethod
    checkStaticMethodNoWait
    checkStaticMethod
)

set(RUNNER_TS_SOURCES "")
foreach(FUNC_NAME ${TEST_FUNCTIONS})
    set(RUNNER_TS "${CMAKE_CURRENT_BINARY_DIR}/runner_${FUNC_NAME}.ts")
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/runner_template.ts.in
        ${RUNNER_TS}
        @ONLY
    )
    list(APPEND RUNNER_TS_SOURCES ${RUNNER_TS})
endforeach()

set(RUNNER_ABC_OUTPUTS "")
foreach(FUNC_NAME ${TEST_FUNCTIONS})
    set(RUNNER_TS "${CMAKE_CURRENT_BINARY_DIR}/runner_${FUNC_NAME}.ts")
    set(RUNNER_ABC "${CMAKE_CURRENT_BINARY_DIR}/runner_${FUNC_NAME}.abc")

    add_custom_command(
        OUTPUT ${RUNNER_ABC}
        COMMAND ${ES2ABC} --module --merge-abc --enable-ets-implements ${RUNNER_TS} --output=${RUNNER_ABC}
        DEPENDS ${RUNNER_TS}
        COMMENT "Generating ${RUNNER_ABC}"
        VERBATIM
    )
    list(APPEND RUNNER_ABC_OUTPUTS ${RUNNER_ABC})
endforeach()

add_custom_target(GenerateRunners DEPENDS ${RUNNER_ABC_OUTPUTS})

panda_ets_interop_js_gtest(ets_interop_js__test_all_ets_promise_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_ts_to_ets.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_ts_to_ets.ets
    TS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_ts_to_ets.ts
    ETS_CONFIG ${ETS_CONFIG}
)

add_dependencies(ets_interop_js__test_all_ets_promise_ts_to_ets GenerateRunners)