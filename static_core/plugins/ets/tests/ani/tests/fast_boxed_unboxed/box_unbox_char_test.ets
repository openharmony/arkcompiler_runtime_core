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

native function testBoxCharNative(): void;
native function testUnboxCharNative(): void;
native function testWithArgsNative(a: char): Object;
native function testPostArrayNative(arr: char[]): void;

function testBoxChar(): void {
    return testBoxCharNative();
}

function testUnboxChar(): void {
    return testUnboxCharNative();
}

class Message {
    msg_optional?: char = c'z';
}

function testWithArgs(a: char): Object {
    return testWithArgsNative(a);
}

function testPostArray(arr: char[]): void {
    return testPostArrayNative(arr);
}
