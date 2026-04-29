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

//! CHECKER      Stackless coro resume with interpreter async context in newly JITted code
//! SKIP_IF      @architecture == "arm32"
//! RUN          entry: "stackless_async_jit_deopt_current.ETSGLOBAL::mainCompileWhileSuspended",
//!              options: "--no-async-jit --compiler-profiling-threshold=0 --compiler-hotness-threshold=1 \
//!                        --compiler-regex='stackless_async_jit_deopt_current.ETSGLOBAL::compileWhileSuspended' \
//!                        --coroutine-impl=stackless --interpreter-type=cpp"
//! METHOD       "stackless_async_jit_deopt_current.ETSGLOBAL::compileWhileSuspended"
//! # callee is compiled
//! EVENT        /Compilation,.*compileWhileSuspended,.*,COMPILED/
//! # caller is resumed in JITted code with interpreter async context (callee suspended and resumed in JITted code)
//! EVENT_NEXT   /DeoptimizationReason,.*compileWhileSuspended,ASYNC_CTX_MISMATCH/
//! EVENT_NOT    /ConvertAsyncCtx,.*compileWhileSuspended/
//! TRUE         EVENT_COUNT(/Compilation,.*compileWhileSuspended,.*,COMPILED/) == 1
//! TRUE         EVENT_COUNT(/DeoptimizationReason,.*compileWhileSuspended,ASYNC_CTX_MISMATCH/) == 1

//! CHECKER      Stackless coro resume with interpreter async context in newly JITted code (irtoc int)
//! SKIP_IF      @architecture == "arm32"
//! RUN          entry: "stackless_async_jit_deopt_current.ETSGLOBAL::mainCompileWhileSuspended",
//!              options: "--no-async-jit --compiler-profiling-threshold=0 --compiler-hotness-threshold=1 \
//!                        --compiler-regex='stackless_async_jit_deopt_current.ETSGLOBAL::compileWhileSuspended' \
//!                        --coroutine-impl=stackless --interpreter-type=irtoc"
//! METHOD       "stackless_async_jit_deopt_current.ETSGLOBAL::compileWhileSuspended"
//! # callee is compiled
//! EVENT        /Compilation,.*compileWhileSuspended,.*,COMPILED/
//! # caller is resumed in JITted code with interpreter async context (callee suspended and resumed in JITted code)
//! EVENT_NEXT   /DeoptimizationReason,.*compileWhileSuspended,ASYNC_CTX_MISMATCH/
//! EVENT_NOT    /ConvertAsyncCtx,.*compileWhileSuspended/
//! TRUE         EVENT_COUNT(/Compilation,.*compileWhileSuspended,.*,COMPILED/) == 1
//! TRUE         EVENT_COUNT(/DeoptimizationReason,.*compileWhileSuspended,ASYNC_CTX_MISMATCH/) == 1

function produce(base: int): Promise<int> {
    return new Promise<int>((resolve) => {
        setTimeout(() => {
            resolve(base + 1)
        }, 1)
    })
}

async function compileWhileSuspended(x: int, depth: int): Promise<int> {
    let variable = x + 10
    let constant = 1

    if (depth == 0) {
        let nested = await compileWhileSuspended(x, depth + 1)
        arktest.assertEQ(variable, x + 10)
        arktest.assertEQ(constant, 1)
        return variable + constant + nested
    }

    let tail = await produce(variable)
    arktest.assertEQ(variable, x + 10)
    arktest.assertEQ(constant, 1)
    arktest.assertEQ(tail, x + 11)
    return variable + constant + tail
}

class Base {
    foo(): int {
        return 1
    }
}

class Child extends Base {
    foo(): int {
        return 2
    }
}

//! CHECKER      Stackless coro resume with mismatched compiled async context in JIT
//! SKIP_IF      @architecture == "arm32"
//! RUN          entry: "stackless_async_jit_deopt_current.ETSGLOBAL::mainDestroyedAndRecompiled",
//!              options: "--no-async-jit --compiler-profiling-threshold=0 --compiler-hotness-threshold=1 \
//!                        --compiler-regex='stackless_async_jit_deopt_current.ETSGLOBAL::recompileWhileSuspended' \
//!                        --coroutine-impl=stackless --interpreter-type=cpp"
//! METHOD       "stackless_async_jit_deopt_current.ETSGLOBAL::recompileWhileSuspended"
//! # interpreted recompileWhileSuspended-0 calls recompileWhileSuspended-1(Base)
//! # recompileWhileSuspended-1 is async-JIT-compiled (JIT1)
//! EVENT        /Compilation,.*recompileWhileSuspended,.*,COMPILED/
//! # JIT1-ted recompileWhileSuspended-1 calls recompileWhileSuspended-2(Child)
//! # recompileWhileSuspended-2(Child) deoptimizes by inline caches
//! EVENT_NEXT   /DeoptimizationReason,.*recompileWhileSuspended,INLINE_IC/
//! # deoptimized interpreted recompileWhileSuspended-2 calls recompileWhileSuspended-3
//! # recompileWhileSuspended-3 is async-JIT-compiled (JIT2)
//! EVENT_NEXT   /Compilation,.*recompileWhileSuspended,.*,COMPILED/
//! # recompileWhileSuspended-3 returns
//! # recompileWhileSuspended-2 is suspended in interpreter, resumed in JIT2 -> interpreter
//! EVENT_NEXT   /DeoptimizationReason,.*recompileWhileSuspended,ASYNC_CTX_MISMATCH/
//! # recompileWhileSuspended-2 returns
//! # recompileWhileSuspended-1 is suspended in JIT1, resumed in JIT2 -> interpreter
//! EVENT_NEXT   /DeoptimizationReason,.*recompileWhileSuspended,ASYNC_CTX_MISMATCH/
//! EVENT_NEXT   /ConvertAsyncCtx,.*recompileWhileSuspended/
//! # recompileWhileSuspended-1 returns
//! # recompileWhileSuspended-0 is suspended in interpreter, resumed in JIT2 -> interpreter
//! EVENT_NEXT   /DeoptimizationReason,.*recompileWhileSuspended,ASYNC_CTX_MISMATCH/
//! # recompileWhileSuspended-0 returns
//! TRUE         EVENT_COUNT(/DeoptimizationReason,.*recompileWhileSuspended,INLINE_IC/) == 1
//! TRUE         EVENT_COUNT(/Compilation,.*recompileWhileSuspended,.*,COMPILED/) == 2
//! TRUE         EVENT_COUNT(/DeoptimizationReason,.*recompileWhileSuspended,ASYNC_CTX_MISMATCH/) == 3
//! TRUE         EVENT_COUNT(/ConvertAsyncCtx,.*recompileWhileSuspended/) == 1

//! CHECKER      Stackless coro resume with mismatched compiled async context in JIT (irtoc int)
//! SKIP_IF      @architecture == "arm32"
//! RUN          entry: "stackless_async_jit_deopt_current.ETSGLOBAL::mainDestroyedAndRecompiled",
//!              options: "--no-async-jit --compiler-profiling-threshold=0 --compiler-hotness-threshold=1 \
//!                        --compiler-regex='stackless_async_jit_deopt_current.ETSGLOBAL::recompileWhileSuspended' \
//!                        --coroutine-impl=stackless --interpreter-type=irtoc"
//! METHOD       "stackless_async_jit_deopt_current.ETSGLOBAL::recompileWhileSuspended"
//! # interpreted recompileWhileSuspended-0 calls recompileWhileSuspended-1(Base)
//! # recompileWhileSuspended-1 is async-JIT-compiled (JIT1)
//! EVENT        /Compilation,.*recompileWhileSuspended,.*,COMPILED/
//! # JIT1-ted recompileWhileSuspended-1 calls recompileWhileSuspended-2(Child)
//! # recompileWhileSuspended-2(Child) deoptimizes by inline caches
//! EVENT_NEXT   /DeoptimizationReason,.*recompileWhileSuspended,INLINE_IC/
//! # deoptimized interpreted recompileWhileSuspended-2 calls recompileWhileSuspended-3
//! # recompileWhileSuspended-3 is async-JIT-compiled (JIT2)
//! EVENT_NEXT   /Compilation,.*recompileWhileSuspended,.*,COMPILED/
//! # recompileWhileSuspended-3 returns
//! # recompileWhileSuspended-2 is suspended in interpreter, resumed in JIT2 -> interpreter
//! EVENT_NEXT   /DeoptimizationReason,.*recompileWhileSuspended,ASYNC_CTX_MISMATCH/
//! # recompileWhileSuspended-2 returns
//! # recompileWhileSuspended-1 is suspended in JIT1, resumed in JIT2 -> interpreter
//! EVENT_NEXT   /DeoptimizationReason,.*recompileWhileSuspended,ASYNC_CTX_MISMATCH/
//! EVENT_NEXT   /ConvertAsyncCtx,.*recompileWhileSuspended/
//! # recompileWhileSuspended-1 returns
//! # recompileWhileSuspended-0 is suspended in interpreter, resumed in JIT2 -> interpreter
//! EVENT_NEXT   /DeoptimizationReason,.*recompileWhileSuspended,ASYNC_CTX_MISMATCH/
//! # recompileWhileSuspended-0 returns
//! TRUE         EVENT_COUNT(/DeoptimizationReason,.*recompileWhileSuspended,INLINE_IC/) == 1
//! TRUE         EVENT_COUNT(/Compilation,.*recompileWhileSuspended,.*,COMPILED/) == 2
//! TRUE         EVENT_COUNT(/DeoptimizationReason,.*recompileWhileSuspended,ASYNC_CTX_MISMATCH/) == 3
//! TRUE         EVENT_COUNT(/ConvertAsyncCtx,.*recompileWhileSuspended/) == 1

async function recompileWhileSuspended(x: int, a: Base, depth: int): Promise<int> {
    let variable = x + 10
    let constant = 1

    let value = 0
    for (let i = 0; i < 20; ++i) {
        value += a.foo()
    }

    let nested = 0
    if (depth < 3) {
        nested = await recompileWhileSuspended(x, depth == 0 ? a : new Child(), depth + 1)
        arktest.assertEQ(variable, x + 10)
        arktest.assertEQ(constant, 1)
        arktest.assertEQ(nested, (variable + constant + 40) * (3 - depth) - (depth == 0 ? 20 : 0) + 1)
    } else {
        nested = await produce(0)
    }
    return variable + constant + value + nested
}

async function mainCompileWhileSuspended(): Promise<void> {
    arktest.assertEQ(await compileWhileSuspended(5, 0), 48)
}

async function mainDestroyedAndRecompiled(): Promise<void> {
    arktest.assertEQ(await recompileWhileSuspended(5, new Base(), 0), 185)
}
