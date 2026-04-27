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

//! CHECKER       Reproduce AOT endless await loop
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*awaitInsideLoop.*'"
//! METHOD        "stackless_await_in_loop.ETSGLOBAL::awaitInsideLoop"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.CompilerEtsAsyncContextCurrent"
//! INST          "Intrinsic.CompilerEtsAwaitResolution"
//! INST          "Intrinsic.CompilerEtsAsyncUnpack"
//! INST          "Intrinsic.CompilerEtsAsyncResolve"
//! INST          "Dispatch"
//! INST_COUNT    "Dispatch", 1
//! ASM_METHOD    "stackless_await_in_loop.ETSGLOBAL::awaitInsideLoop"
//! ASM_INST      "Intrinsic.CompilerEtsAsyncContextCurrent"
//! ASM_INST      "CallEntrypoint: EtsAsyncContextGetCurrent"
//! ASM_INST      "CallEntrypoint: EtsCompilerAwaitResolution"
//! ASM_INST      "CallEntrypoint: EtsAsyncUnpack"
//! ASM_INST      "CallEntrypoint: EtsAsyncResolve"

let keepWaiting: boolean = true;

function resolvedStep(): Promise<void> {
    return Promise.resolve();
}

async function awaitInsideLoop(): Promise<void> {
    while (keepWaiting) {
        await resolvedStep();
    }
}

function main(): void {
    awaitInsideLoop();
}
