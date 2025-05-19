/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

function callbackJsFunctionEts(fn: (x: number) => number): number {
    return fn(0x55aa);
}

function callbackFunctionEts(fn: Function) {
    return fn.unsafeCall(0x55aa);
}

function callbackFunctionEtsTwoParam(fn: Function) {
    return fn.unsafeCall(0x55aa, 1);
}

function callbackFunctionEtsTestSum(sumFunc: Function): boolean {
    assertTrue(sumFunc.unsafeCall() === 0);
    assertTrue(sumFunc.unsafeCall(1) === 1);
    assertTrue(sumFunc.unsafeCall(1, 2) === 3);
    assertTrue(sumFunc.unsafeCall(1, 2, 3) === 6);
    return true;
}