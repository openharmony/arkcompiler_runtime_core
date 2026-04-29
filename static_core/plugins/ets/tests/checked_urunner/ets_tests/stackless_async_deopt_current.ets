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

//! CHECKER      Stackless coro non-destroying deopt before suspend in AOT
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: ""
//! METHOD       "stackless_async_deopt_current.ETSGLOBAL::currentOrdinaryBeforeSuspend"
//! RUN          entry: "stackless_async_deopt_current.ETSGLOBAL::mainBefore",
//!              options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=cpp"
//! # throw + deoptimize
//! EVENT        /Exception,.*currentOrdinaryBeforeSuspend,.*,THROW/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_current.ETSGLOBAL::currentOrdinaryBeforeSuspend/
//! # resume: deoptimize because of interpreter async context
//! EVENT_NEXT   /DeoptimizationReason,.*currentOrdinaryBeforeSuspend,ASYNC_CTX_MISMATCH/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_current.ETSGLOBAL::currentOrdinaryBeforeSuspend/
//! EVENT_NOT    /ConvertAsyncCtx,.*currentOrdinaryBeforeSuspend/
//! TRUE         EVENT_COUNT(/DeoptimizationReason,.*currentOrdinaryBeforeSuspend,ASYNC_CTX_MISMATCH/) == 1
//! TRUE         EVENT_COUNT(/Deoptimization,.*currentOrdinaryBeforeSuspend/) == 2

//! CHECKER      Stackless coro non-destroying deopt before suspend in AOT (irtoc int)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: ""
//! METHOD       "stackless_async_deopt_current.ETSGLOBAL::currentOrdinaryBeforeSuspend"
//! RUN          entry: "stackless_async_deopt_current.ETSGLOBAL::mainBefore",
//!              options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=irtoc"
//! # throw + deoptimize
//! EVENT        /Exception,.*currentOrdinaryBeforeSuspend,.*,THROW/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_current.ETSGLOBAL::currentOrdinaryBeforeSuspend/
//! # resume: deoptimize because of interpreter async context
//! EVENT_NEXT   /DeoptimizationReason,.*currentOrdinaryBeforeSuspend,ASYNC_CTX_MISMATCH/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_current.ETSGLOBAL::currentOrdinaryBeforeSuspend/
//! EVENT_NOT    /ConvertAsyncCtx,.*currentOrdinaryBeforeSuspend/
//! TRUE         EVENT_COUNT(/DeoptimizationReason,.*currentOrdinaryBeforeSuspend,ASYNC_CTX_MISMATCH/) == 1
//! TRUE         EVENT_COUNT(/Deoptimization,.*currentOrdinaryBeforeSuspend/) == 2

function produce(base: int): Promise<int> {
    return new Promise<int>((resolve) => {
        setTimeout(() => {
            resolve(base + 1)
        }, 1)
    })
}

async function currentOrdinaryBeforeSuspend(x: int): Promise<int> {
    let variable = x + 10
    let constant = 1
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

//! CHECKER      Stackless coro non-destroying deopt after suspend in AOT
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: ""
//! METHOD       "stackless_async_deopt_current.ETSGLOBAL::currentOrdinaryAfterSuspend"
//! RUN          entry: "stackless_async_deopt_current.ETSGLOBAL::mainAfter",
//!              options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=cpp"
//! # throw + deoptimize
//! EVENT        /Exception,.*currentOrdinaryAfterSuspend,.*,THROW/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_current.ETSGLOBAL::currentOrdinaryAfterSuspend/
//! # resume: deoptimize because of interpreter async context
//! EVENT_NEXT   /DeoptimizationReason,.*currentOrdinaryAfterSuspend,ASYNC_CTX_MISMATCH/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_current.ETSGLOBAL::currentOrdinaryAfterSuspend/
//! EVENT_NOT    /ConvertAsyncCtx,.*currentOrdinaryAfterSuspend/
//! TRUE         EVENT_COUNT(/DeoptimizationReason,.*currentOrdinaryAfterSuspend,ASYNC_CTX_MISMATCH/) == 1
//! TRUE         EVENT_COUNT(/Deoptimization,.*currentOrdinaryAfterSuspend/) == 2

//! CHECKER      Stackless coro non-destroying deopt after suspend in AOT (irtoc int)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: ""
//! METHOD       "stackless_async_deopt_current.ETSGLOBAL::currentOrdinaryAfterSuspend"
//! RUN          entry: "stackless_async_deopt_current.ETSGLOBAL::mainAfter",
//!              options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=irtoc"
//! # throw + deoptimize
//! EVENT        /Exception,.*currentOrdinaryAfterSuspend,.*,THROW/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_current.ETSGLOBAL::currentOrdinaryAfterSuspend/
//! # resume: deoptimize because of interpreter async context
//! EVENT_NEXT   /DeoptimizationReason,.*currentOrdinaryAfterSuspend,ASYNC_CTX_MISMATCH/
//! EVENT_NEXT   /Deoptimization,stackless_async_deopt_current.ETSGLOBAL::currentOrdinaryAfterSuspend/
//! EVENT_NOT    /ConvertAsyncCtx,.*currentOrdinaryAfterSuspend/
//! TRUE         EVENT_COUNT(/DeoptimizationReason,.*currentOrdinaryAfterSuspend,ASYNC_CTX_MISMATCH/) == 1
//! TRUE         EVENT_COUNT(/Deoptimization,.*currentOrdinaryAfterSuspend/) == 2

async function currentOrdinaryAfterSuspend(x: int): Promise<int> {
    let variable = x + 10
    let constant = 1
    let caught = 0

    let resumed = await produce(variable)
    arktest.assertEQ(variable, x + 10)
    arktest.assertEQ(constant, 1)
    arktest.assertEQ(caught, 0)
    arktest.assertEQ(resumed, x + 11)

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
    arktest.assertEQ(await currentOrdinaryBeforeSuspend(5), 33)
}

async function mainAfter(): Promise<void> {
    arktest.assertEQ(await currentOrdinaryAfterSuspend(5), 50)
}
