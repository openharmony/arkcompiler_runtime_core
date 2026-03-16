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

native function testBoxLongNative(): void;
native function testUnboxLongNative(): void;
native function testWithArgsNative(a: long): Object;
native function testPostArrayNative(arr: long[]): void;

function testBoxLong(): void {
    return testBoxLongNative();
}

function testUnboxLong(): void {
    return testUnboxLongNative();
}

class Message {
    score_optional?: long = 123456789;
}

function testWithArgs(a: long): Object {
    return testWithArgsNative(a);
}

function testPostArray(arr: long[]): void {
    return testPostArrayNative(arr);
}
