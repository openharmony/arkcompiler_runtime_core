/**
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

let etsVm = globalThis.gtest.etsVm;

let staFun0 = etsVm.getFunction('Lhybrid_stack_ets_to_ts/ETSGLOBAL;', 'staFun0');
let testPropertyAccess = etsVm.getFunction('Lhybrid_stack_ets_to_ts/ETSGLOBAL;', 'testPropertyAccess');
let testFunctionCall = etsVm.getFunction('Lhybrid_stack_ets_to_ts/ETSGLOBAL;', 'testFunctionCall');

export function jsCallback(): boolean {
    return true;
}

export let jsValue = 42;

export function dyFun1(): void {
    staFun0();
}

function main(): void {
    testPropertyAccess();
    testFunctionCall();
}

main();
