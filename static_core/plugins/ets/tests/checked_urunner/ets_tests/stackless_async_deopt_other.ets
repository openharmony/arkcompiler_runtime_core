/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//! DEFINE_FRONTEND_OPTIONS DEFAULT ARGS
//! RUN_FRONTEND            options: "--stackless-coros=true"

//! CHECKER      Stackless coro non-destroying deopt in another invocation before suspend in AOT
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: ""
//! METHOD       "stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryBeforeSuspend"
//! RUN          entry: "stackless_async_deopt_other.ETSGLOBAL::mainBefore",
//!              options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=cpp"
//! # throw + deoptimize
//! EVENT        /Exception,.*otherOrdinaryBeforeSuspend,.*,THROW/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryBeforeSuspend/
//! # callee resume: deoptimize because of interpreter async context
//! EVENT_NEXT   /DeoptimizationReason,.*otherOrdinaryBeforeSuspend,ASYNC_CTX_MISMATCH/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryBeforeSuspend/
//! EVENT_NOT    /ConvertAsyncCtx,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryBeforeSuspend/
//! # resuming caller in compiled code
//! TRUE         EVENT_COUNT(/DeoptimizationReason,.*otherOrdinaryBeforeSuspend,ASYNC_CTX_MISMATCH/) == 1
//! TRUE         EVENT_COUNT(/Deoptimization,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryBeforeSuspend/) == 2

//! CHECKER      Stackless coro non-destroying deopt in another invocation before suspend in AOT (irtoc int)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: ""
//! METHOD       "stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryBeforeSuspend"
//! RUN          entry: "stackless_async_deopt_other.ETSGLOBAL::mainBefore",
//!              options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=irtoc"
//! # throw + deoptimize
//! EVENT        /Exception,.*otherOrdinaryBeforeSuspend,.*,THROW/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryBeforeSuspend/
//! # callee resume: deoptimize because of interpreter async context
//! EVENT_NEXT   /DeoptimizationReason,.*otherOrdinaryBeforeSuspend,ASYNC_CTX_MISMATCH/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryBeforeSuspend/
//! EVENT_NOT    /ConvertAsyncCtx,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryBeforeSuspend/
//! # resuming caller in compiled code
//! TRUE         EVENT_COUNT(/DeoptimizationReason,.*otherOrdinaryBeforeSuspend,ASYNC_CTX_MISMATCH/) == 1
//! TRUE         EVENT_COUNT(/Deoptimization,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryBeforeSuspend/) == 2

function produce(base: int): Promise<int> {
    return new Promise<int>((resolve) => {
        setTimeout(() => {
            resolve(base + 1)
        }, 1)
    })
}

async function otherOrdinaryBeforeSuspend(x: int, depth: int): Promise<int> {
    let variable = x + 10
    let constant = 1

    if (depth == 0) {
        let nested = await otherOrdinaryBeforeSuspend(x, 1)
        arktest.assertEQ(variable, x + 10)
        arktest.assertEQ(constant, 1)
        arktest.assertEQ(nested, variable + constant + 1 + x + 11)
        let tail = await produce(variable)
        arktest.assertEQ(variable, x + 10)
        arktest.assertEQ(constant, 1)
        arktest.assertEQ(tail, x + 11)
        return variable + constant + nested + tail
    }

    let caught = 0
    try {
        throw new Error("before")
    } catch (e) {
        caught = 1
        arktest.assertEQ(variable, x + 10)
        arktest.assertEQ(constant, 1)
    }

    arktest.assertEQ(caught, 1)
    let tail = await produce(variable)
    arktest.assertEQ(variable, x + 10)
    arktest.assertEQ(constant, 1)
    arktest.assertEQ(tail, x + 11)
    return variable + constant + caught + tail
}

//! CHECKER      Stackless coro non-destroying deopt in another invocation after suspend in AOT
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: ""
//! METHOD       "stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryAfterSuspend"
//! RUN          entry: "stackless_async_deopt_other.ETSGLOBAL::mainAfter",
//!              options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=cpp"
//! # throw + deoptimize
//! EVENT        /Exception,.*otherOrdinaryAfterSuspend,.*,THROW/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryAfterSuspend/
//! # callee resume: deoptimize because of interpreter async context
//! EVENT_NEXT   /DeoptimizationReason,.*otherOrdinaryAfterSuspend,ASYNC_CTX_MISMATCH/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryAfterSuspend/
//! EVENT_NOT    /ConvertAsyncCtx,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryAfterSuspend/
//! # resuming caller in compiled code
//! TRUE         EVENT_COUNT(/DeoptimizationReason,.*otherOrdinaryAfterSuspend,ASYNC_CTX_MISMATCH/) == 1
//! TRUE         EVENT_COUNT(/Deoptimization,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryAfterSuspend/) == 2

//! CHECKER      Stackless coro non-destroying deopt in another invocation after suspend in AOT (irtoc int)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: ""
//! METHOD       "stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryAfterSuspend"
//! RUN          entry: "stackless_async_deopt_other.ETSGLOBAL::mainAfter",
//!              options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=irtoc"
//! # throw + deoptimize
//! EVENT        /Exception,.*otherOrdinaryAfterSuspend,.*,THROW/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryAfterSuspend/
//! # callee resume: deoptimize because of interpreter async context
//! EVENT_NEXT   /DeoptimizationReason,.*otherOrdinaryAfterSuspend,ASYNC_CTX_MISMATCH/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryAfterSuspend/
//! EVENT_NOT    /ConvertAsyncCtx,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryAfterSuspend/
//! # resuming caller in compiled code
//! TRUE         EVENT_COUNT(/DeoptimizationReason,.*otherOrdinaryAfterSuspend,ASYNC_CTX_MISMATCH/) == 1
//! TRUE         EVENT_COUNT(/Deoptimization,stackless_async_deopt_other.ETSGLOBAL::otherOrdinaryAfterSuspend/) == 2

async function otherOrdinaryAfterSuspend(x: int, depth: int): Promise<int> {
    let variable = x + 10
    let constant = 1
    let caught = 0

    let resumed = await produce(variable)
    arktest.assertEQ(variable, x + 10)
    arktest.assertEQ(constant, 1)
    arktest.assertEQ(caught, 0)
    arktest.assertEQ(resumed, x + 11)

    if (depth == 0) {
        let nested = await otherOrdinaryAfterSuspend(x, 1)
        arktest.assertEQ(variable, x + 10)
        arktest.assertEQ(constant, 1)
        arktest.assertEQ(caught, 0)
        arktest.assertEQ(resumed, x + 11)
        arktest.assertEQ(nested, variable + constant + 1 + resumed + x + 12)
        let tail = await produce(resumed)
        arktest.assertEQ(variable, x + 10)
        arktest.assertEQ(constant, 1)
        arktest.assertEQ(caught, 0)
        arktest.assertEQ(resumed, x + 11)
        arktest.assertEQ(tail, x + 12)
        return variable + constant + nested + resumed + tail
    }

    try {
        throw new Error("after")
    } catch (e) {
        caught = 1
        arktest.assertEQ(variable, x + 10)
        arktest.assertEQ(constant, 1)
        arktest.assertEQ(resumed, x + 11)
    }

    arktest.assertEQ(caught, 1)
    let tail = await produce(resumed)
    arktest.assertEQ(variable, x + 10)
    arktest.assertEQ(constant, 1)
    arktest.assertEQ(resumed, x + 11)
    arktest.assertEQ(tail, x + 12)
    return variable + constant + caught + resumed + tail
}

async function mainBefore(): Promise<void> {
    arktest.assertEQ(await otherOrdinaryBeforeSuspend(5, 0), 65)
}

async function mainAfter(): Promise<void> {
    arktest.assertEQ(await otherOrdinaryAfterSuspend(5, 0), 99)
}
