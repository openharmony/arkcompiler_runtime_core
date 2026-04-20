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

//! CHECKER      Check stackless async method is skipped for OSR
//! SKIP_IF      @architecture != "arm64"
//! RUN          options: "--no-async-jit --compiler-hotness-threshold=1 --compiler-enable-osr=true \
//!                        --coroutine-impl=stackless --interpreter-type=cpp",
//!              entry: "stackless_async_osr_skip.ETSGLOBAL::main"
//! EVENT        /Compilation,stackless_async_osr_skip.ETSGLOBAL::asyncHotLoop,.*,COMPILED/
//! EVENT_NOT    /OsrEntry,stackless_async_osr_skip.ETSGLOBAL::asyncHotLoop,.*,SUCCESS/

//! CHECKER      Check stackless async method is skipped for OSR (irtoc int)
//! SKIP_IF      @architecture != "arm64"
//! RUN          options: "--no-async-jit --compiler-hotness-threshold=1 --compiler-enable-osr=true \
//!                        --coroutine-impl=stackless --interpreter-type=irtoc",
//!              entry: "stackless_async_osr_skip.ETSGLOBAL::main"
//! EVENT        /Compilation,stackless_async_osr_skip.ETSGLOBAL::asyncHotLoop,.*,COMPILED/
//! EVENT_NOT    /OsrEntry,stackless_async_osr_skip.ETSGLOBAL::asyncHotLoop,.*,SUCCESS/

function produce(base: int): Promise<int> {
    return new Promise<int>((resolve) => {
        setTimeout(() => {
            resolve(base + 1)
        }, 1)
    })
}

async function asyncHotLoop(x: int): Promise<int> {
    let sum = 0
    for (let i = 0; i < 100; ++i) {
        sum += x + i
    }
    let tail = await produce(sum)
    return tail
}

async function main(): Promise<void> {
    arktest.assertEQ(await asyncHotLoop(0), 4951)
    arktest.assertEQ(await asyncHotLoop(1), 5051)
    arktest.assertEQ(await asyncHotLoop(2), 5151)
}
