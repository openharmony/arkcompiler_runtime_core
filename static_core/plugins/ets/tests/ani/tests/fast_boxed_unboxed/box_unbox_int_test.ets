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

native function testBoxIntNative(): void;
native function testUnboxIntNative(): void;
native function testWithArgsNative(a: int): Object;
native function testPostArrayNative(arr: int[]): void;

function testBoxInt(): void {
    return testBoxIntNative();
}

function testUnboxInt(): void {
    return testUnboxIntNative();
}

class Message {
    score_optional?: int = 1234;
}

function testWithArgs(a: int): Object {
    return testWithArgsNative(a);
}

function testPostArray(arr: int[]): void {
    return testPostArrayNative(arr);
}
