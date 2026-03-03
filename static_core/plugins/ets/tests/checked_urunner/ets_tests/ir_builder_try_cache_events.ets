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

//! CHECKER      IrBuilder try-cache is used only for try/catch path (JIT)
//! RUN          force_jit: true, options: "--compiler-regex='ir_builder_try_cache_events\\.ETSGLOBAL::(main|withTry|withoutTry)'", \
//!              entry: "ir_builder_try_cache_events.ETSGLOBAL::main"
//! EVENT        /IrBuilderTryCache,ir_builder_try_cache_events\.ETSGLOBAL::withTry,PUT/
//! EVENT_NEXT   /IrBuilderTryCache,ir_builder_try_cache_events\.ETSGLOBAL::withTry,GET/
//! EVENT_NOT    /IrBuilderTryCache,ir_builder_try_cache_events\.ETSGLOBAL::withoutTry,/

function withTry(arr: int[], idx: int): int {
    try {
        return arr[idx];
    } catch (e) {
        return -1;
    }
}

function withoutTry(arr: int[], idx: int): int {
    return arr[idx];
}

function main(): int {
    let arr: int[] = [1, 2, 3];
    let withTryResult = withTry(arr, 1);
    let withoutTryResult = withoutTry(arr, 0);
    arktest.assertEQ(withTryResult + withoutTryResult, 3);
    return 0;
}
