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

// NOTE: Enable disabled checkers #34101
//! DEFINE_FRONTEND_OPTIONS  DEFAULT ARGS
//! RUN_FRONTEND            options: "--stackless-coros=true"

//! DISABLED_CHECKER      Check stackless async compiled bridge in JIT
//! SKIP_IF      @architecture != "x64"
//! RUN          force_jit: true,
//!              options: "--compiler-regex='stackless_async_compiled_bridge.*(Check|start)' --compiler-ignore-failures=false",
//!              entry: "stackless_async_compiled_bridge.ETSGLOBAL::main"
//! METHOD       "stackless_async_compiled_bridge.ETSGLOBAL::resumeCheck"
//! PASS_AFTER   "IrBuilder"
//! INST         "Dispatch"
//! INST         "SaveStateSuspend"
//! METHOD       "stackless_async_compiled_bridge.ETSGLOBAL::rejectCheck"
//! PASS_AFTER   "IrBuilder"
//! INST         "Dispatch"
//! INST         "SaveStateSuspend"
//! METHOD       "stackless_async_compiled_bridge.ETSGLOBAL::floatResumeCheck"
//! PASS_AFTER   "IrBuilder"
//! INST         "Dispatch"
//! INST         "SaveStateSuspend"
//! METHOD       "stackless_async_compiled_bridge.ETSGLOBAL::loopResumeCheck"
//! PASS_AFTER   "IrBuilder"
//! INST         "Dispatch"
//! INST         "SaveStateSuspend"
//! METHOD       "stackless_async_compiled_bridge.ETSGLOBAL::start"
//! PASS_AFTER   "Inline"
//! INST_NOT     /Call.*Inlined.*resumeCheck/
//! INST_NOT     /Call.*Inlined.*rejectCheck/
//! INST_NOT     /Call.*Inlined.*floatResumeCheck/
//! INST_NOT     /Call.*Inlined.*loopResumeCheck/
//! ASM_METHOD   "stackless_async_compiled_bridge.ETSGLOBAL::start"
//! ASM_INST     "Intrinsic.StdPromiseAwait"
//! ASM_INST     "CallEntrypoint: EtsStacklessInitAsyncContext"

//! CHECKER      Check stackless async compiled bridge in AOT
//! SKIP_IF      @architecture != "x64"
//! RUN_PAOC     options: "--compiler-regex='stackless_async_compiled_bridge.*(Check|start)' --compiler-ignore-failures=false"
//! METHOD       "stackless_async_compiled_bridge.ETSGLOBAL::resumeCheck"
//! PASS_AFTER   "IrBuilder"
//! INST         "Dispatch"
//! INST         "SaveStateSuspend"
//! METHOD       "stackless_async_compiled_bridge.ETSGLOBAL::rejectCheck"
//! PASS_AFTER   "IrBuilder"
//! INST         "Dispatch"
//! INST         "SaveStateSuspend"
//! METHOD       "stackless_async_compiled_bridge.ETSGLOBAL::floatResumeCheck"
//! PASS_AFTER   "IrBuilder"
//! INST         "Dispatch"
//! INST         "SaveStateSuspend"
//! METHOD       "stackless_async_compiled_bridge.ETSGLOBAL::loopResumeCheck"
//! PASS_AFTER   "IrBuilder"
//! INST         "Dispatch"
//! INST         "SaveStateSuspend"
//! METHOD       "stackless_async_compiled_bridge.ETSGLOBAL::start"
//! PASS_AFTER   "Inline"
//! INST_NOT     /Call.*Inlined.*resumeCheck/
//! INST_NOT     /Call.*Inlined.*rejectCheck/
//! INST_NOT     /Call.*Inlined.*floatResumeCheck/
//! INST_NOT     /Call.*Inlined.*loopResumeCheck/
//! ASM_METHOD   "stackless_async_compiled_bridge.ETSGLOBAL::start"
//! ASM_INST     "Intrinsic.StdPromiseAwait"
//! ASM_INST     "CallEntrypoint: EtsStacklessInitAsyncContext"

class Dummy {
    value: int

    constructor(value: int) {
        this.value = value
    }
}

function produce(base: int): Promise<int> {
    return new Promise<int>((resolve) => {
        setTimeout(() => {
            resolve(base + 1)
        }, 1)
    })
}

function throwingProduce(): Promise<int> {
    return new Promise<int>((_, reject) => {
        setTimeout(() => {
            reject(new Error("test"))
        }, 1)
    })
}

// Check that state is properly saved and restored across suspension points when promise is fulfilled.
async function resumeCheck(x: int): Promise<int> {
    let base = x
    let checkpoint = 1
    let dummy = new Dummy(2)
    arktest.assertEQ(base, x)
    arktest.assertEQ(dummy.value, 2)

    let first = await produce(base)
    arktest.assertEQ(first, base + 1)
    arktest.assertEQ(checkpoint, 1)
    arktest.assertEQ(base, x)
    arktest.assertEQ(dummy.value, 2)

    checkpoint = 2
    dummy.value = 3
    let second = await produce(dummy.value)
    arktest.assertEQ(checkpoint, 2)
    arktest.assertEQ(base, x)
    arktest.assertEQ(first, base + 1)
    arktest.assertEQ(dummy.value, 3)
    arktest.assertEQ(second, 4)

    return base + second
}

// Check that state is properly saved and restored across suspension points when promise is rejected.
async function rejectCheck(x: int): Promise<int> {
    let base = x
    let checkpoint = 1
    let dummy = new Dummy(2)
    let caught = false

    try {
        await throwingProduce()
        arktest.assertEQ(0, 1)
    } catch (e) {
        caught = true
        arktest.assertEQ(checkpoint, 1)
        arktest.assertEQ(base, x)
        arktest.assertEQ(dummy.value, 2)
        checkpoint = 2
        dummy.value = 3
    }

    arktest.assertEQ(caught, true)
    arktest.assertEQ(checkpoint, 2)
    return dummy.value + base
}

// Check that fp registers are properly saved and restored across suspension points.
async function floatResumeCheck(x: int): Promise<int> {
    let base = x
    let f: float = 1.0f
    let d: double = 2.0

    let first = await produce(base)
    arktest.assertEQ(first, base + 1)
    arktest.assertEQ(f, 1.0f)
    arktest.assertEQ(d, 2.0)

    f = 2.0f
    d = 3.0
    let second = await produce(2)
    arktest.assertEQ(second, 3)
    arktest.assertEQ(f, 2.0f)
    arktest.assertEQ(d, 3.0)

    return second + base
}

// Check that registers used in loop are properly saved and restored across suspension points in each iteration.
async function loopResumeCheck(x: int): Promise<int> {
    let v = x
    let sum = 0
    let dummy = new Dummy(1)

    for (let i = 0; i < 2; i++) {
        let current = await produce(v + i)
        sum = sum + current
        dummy.value = dummy.value + 1
        arktest.assertEQ(v, x)
    }

    arktest.assertEQ(sum, 2 * v + 3)
    arktest.assertEQ(dummy.value, 3)
    return sum
}

async function start() {
    const result0 = await resumeCheck(0)
    arktest.assertEQ(result0, 4)

    const result1 = await resumeCheck(1)
    arktest.assertEQ(result1, 5)

    const reject0 = await rejectCheck(1)
    arktest.assertEQ(reject0, 4)

    const reject1 = await rejectCheck(2)
    arktest.assertEQ(reject1, 5)

    const float0 = await floatResumeCheck(0)
    arktest.assertEQ(float0, 3)

    const float1 = await floatResumeCheck(1)
    arktest.assertEQ(float1, 4)

    const loop0 = await loopResumeCheck(0)
    arktest.assertEQ(loop0, 3)

    const loop1 = await loopResumeCheck(1)
    arktest.assertEQ(loop1, 5)
}

function main(): void {
    waitForCompletion(start)
}
