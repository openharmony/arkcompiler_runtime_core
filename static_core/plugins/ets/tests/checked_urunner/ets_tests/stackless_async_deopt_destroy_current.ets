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

//! CHECKER           Stackless coro destroying deopt before suspend in AOT-PGO
//! SKIP_IF           @architecture == "arm32"
//! RUN_PGO_PROF      entry: "stackless_async_deopt_destroy_current.ETSGLOBAL::mainProfileABefore",
//!                   options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=cpp"
//! RUN_PGO_PAOC      options: ""
//! METHOD            "stackless_async_deopt_destroy_current.ETSGLOBAL::currentDestroyedBeforeSuspend"
//! PASS_AFTER        "Inline"
//! INST              /DeoptimizeIf INLINE_IC/
//! RUN               entry: "stackless_async_deopt_destroy_current.ETSGLOBAL::mainRunBBefore",
//!                   options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=cpp"
//! # deoptimize by inline caches
//! EVENT             /DeoptimizationReason,.*currentDestroyedBeforeSuspend,INLINE_IC/
//! # no further deoptimization as we don't come back to destroyed compiled code
//! EVENT_NOT         /DeoptimizationReason,.*currentDestroyedBeforeSuspend,ASYNC_CTX_MISMATCH/
//! EVENT_NOT         /ConvertAsyncCtx,.*currentDestroyedBeforeSuspend/
//! TRUE              EVENT_COUNT(/DeoptimizationReason,.*currentDestroyedBeforeSuspend,INLINE_IC/) == 1

//! CHECKER           Stackless coro destroying deopt before suspend in AOT-PGO (irtoc int)
//! SKIP_IF           @architecture == "arm32"
//! RUN_PGO_PROF      entry: "stackless_async_deopt_destroy_current.ETSGLOBAL::mainProfileABefore",
//!                   options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=irtoc"
//! RUN_PGO_PAOC      options: ""
//! METHOD            "stackless_async_deopt_destroy_current.ETSGLOBAL::currentDestroyedBeforeSuspend"
//! PASS_AFTER        "Inline"
//! INST              /DeoptimizeIf INLINE_IC/
//! RUN               entry: "stackless_async_deopt_destroy_current.ETSGLOBAL::mainRunBBefore",
//!                   options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=irtoc"
//! # deoptimize by inline caches
//! EVENT             /DeoptimizationReason,.*currentDestroyedBeforeSuspend,INLINE_IC/
//! # no further deoptimization as we don't come back to destroyed compiled code
//! EVENT_NOT         /DeoptimizationReason,.*currentDestroyedBeforeSuspend,ASYNC_CTX_MISMATCH/
//! EVENT_NOT         /ConvertAsyncCtx,.*currentDestroyedBeforeSuspend/
//! TRUE              EVENT_COUNT(/DeoptimizationReason,.*currentDestroyedBeforeSuspend,INLINE_IC/) == 1

function produce(base: int): Promise<int> {
    return new Promise<int>((resolve) => {
        setTimeout(() => {
            resolve(base + 1)
        }, 1)
    })
}

class A {
    foo(): int {
        return 1
    }
}

class B extends A {
    foo(): int {
        return 2
    }
}

async function currentDestroyedBeforeSuspend(x: int, a: A, expected: int): Promise<int> {
    let variable = x + 10
    let constant = 1

    let value = 0
    for (let i = 0; i < 20; ++i) {
        value += a.foo()
    }
    arktest.assertEQ(value, expected)
    arktest.assertEQ(variable, x + 10)
    arktest.assertEQ(constant, 1)

    let tail = await produce(variable)
    arktest.assertEQ(variable, x + 10)
    arktest.assertEQ(constant, 1)
    arktest.assertEQ(tail, x + 11)
    return variable + constant + value + tail
}

//! CHECKER           Stackless coro destroying deopt after suspend in AOT-PGO
//! SKIP_IF           @architecture == "arm32"
//! RUN_PGO_PROF      entry: "stackless_async_deopt_destroy_current.ETSGLOBAL::mainProfileAAfter",
//!                   options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=cpp"
//! RUN_PGO_PAOC      options: ""
//! METHOD            "stackless_async_deopt_destroy_current.ETSGLOBAL::currentDestroyedAfterSuspend"
//! PASS_AFTER        "Inline"
//! INST              /DeoptimizeIf INLINE_IC/
//! RUN               entry: "stackless_async_deopt_destroy_current.ETSGLOBAL::mainRunBAfter",
//!                   options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=cpp"
//! # deoptimize by inline caches
//! EVENT             /DeoptimizationReason,.*currentDestroyedAfterSuspend,INLINE_IC/
//! # no further deoptimization as we don't come back to destroyed compiled code
//! EVENT_NOT         /DeoptimizationReason,.*currentDestroyedAfterSuspend,ASYNC_CTX_MISMATCH/
//! EVENT_NOT         /ConvertAsyncCtx,stackless_async_deopt_destroy_current.ETSGLOBAL::currentDestroyedAfterSuspend/
//! TRUE              EVENT_COUNT(/DeoptimizationReason,.*currentDestroyedAfterSuspend,INLINE_IC/) == 1

//! CHECKER           Stackless coro destroying deopt after suspend in AOT-PGO (irtoc int)
//! SKIP_IF           @architecture == "arm32"
//! RUN_PGO_PROF      entry: "stackless_async_deopt_destroy_current.ETSGLOBAL::mainProfileAAfter",
//!                   options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=irtoc"
//! RUN_PGO_PAOC      options: ""
//! METHOD            "stackless_async_deopt_destroy_current.ETSGLOBAL::currentDestroyedAfterSuspend"
//! PASS_AFTER        "Inline"
//! INST              /DeoptimizeIf INLINE_IC/
//! RUN               entry: "stackless_async_deopt_destroy_current.ETSGLOBAL::mainRunBAfter",
//!                   options: "--compiler-enable-jit=false --coroutine-impl=stackless --interpreter-type=irtoc"
//! # deoptimize by inline caches
//! EVENT             /DeoptimizationReason,.*currentDestroyedAfterSuspend,INLINE_IC/
//! # no further deoptimization as we don't come back to destroyed compiled code
//! EVENT_NOT         /DeoptimizationReason,.*currentDestroyedAfterSuspend,ASYNC_CTX_MISMATCH/
//! EVENT_NOT         /ConvertAsyncCtx,stackless_async_deopt_destroy_current.ETSGLOBAL::currentDestroyedAfterSuspend/
//! TRUE              EVENT_COUNT(/DeoptimizationReason,.*currentDestroyedAfterSuspend,INLINE_IC/) == 1

async function currentDestroyedAfterSuspend(x: int, a: A, expected: int): Promise<int> {
    let variable = x + 10
    let constant = 1

    let resumed = await produce(variable)
    arktest.assertEQ(variable, x + 10)
    arktest.assertEQ(constant, 1)
    arktest.assertEQ(resumed, x + 11)

    let value = 0
    for (let i = 0; i < 20; ++i) {
        value += a.foo()
    }
    arktest.assertEQ(value, expected)

    let tail = await produce(resumed)
    arktest.assertEQ(variable, x + 10)
    arktest.assertEQ(constant, 1)
    arktest.assertEQ(resumed, x + 11)
    arktest.assertEQ(tail, x + 12)
    return variable + constant + value + resumed + tail
}

async function mainProfileABefore(): Promise<void> {
    arktest.assertEQ(await currentDestroyedBeforeSuspend(5, new A(), 20), 52)
}

async function mainRunBBefore(): Promise<void> {
    arktest.assertEQ(await currentDestroyedBeforeSuspend(5, new B(), 40), 72)
}

async function mainProfileAAfter(): Promise<void> {
    arktest.assertEQ(await currentDestroyedAfterSuspend(5, new A(), 20), 69)
}

async function mainRunBAfter(): Promise<void> {
    arktest.assertEQ(await currentDestroyedAfterSuspend(5, new B(), 40), 89)
}

