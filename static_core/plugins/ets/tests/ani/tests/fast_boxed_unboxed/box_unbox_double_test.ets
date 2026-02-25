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

native function testBoxDoubleNative(): void;
native function testUnboxDoubleNative(): void;
native function testWithArgsNative(a: double): Object;
native function testPostArrayNative(arr: double[]): void;

function testBoxDouble(): void {
    return testBoxDoubleNative();
}

function testUnboxDouble(): void {
    return testUnboxDoubleNative();
}

class Message {
    score_optional?: double = 1.23;
}

function testWithArgs(a: double): Object {
    return testWithArgsNative(a);
}

function testPostArray(arr: double[]): void {
    return testPostArrayNative(arr);
}
