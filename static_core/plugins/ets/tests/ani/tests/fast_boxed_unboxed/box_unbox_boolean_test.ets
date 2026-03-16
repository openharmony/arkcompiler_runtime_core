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

native function testBoxBooleanNative(): void;
native function testUnboxBooleanNative(): void;
native function testWithArgsNative(a: boolean): Object;
native function testPostArrayNative(arr: boolean[]): void;

function testBoxBoolean(): void {
    return testBoxBooleanNative();
}

function testUnboxBoolean(): void {
    return testUnboxBooleanNative();
}

class Message {
    gender_optional?: boolean = false;
}

function testWithArgs(a: boolean): Object {
    return testWithArgsNative(a);
}

function testPostArray(arr: boolean[]): void {
    return testPostArrayNative(arr);
}
