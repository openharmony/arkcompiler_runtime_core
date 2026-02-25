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

native function testBoxShortNative(): void;
native function testUnboxShortNative(): void;
native function testWithArgsNative(a: short): Object;
native function testPostArrayNative(arr: short[]): void;

function testBoxShort(): void {
    return testBoxShortNative();
}

function testUnboxShort(): void {
    return testUnboxShortNative();
}

class Message {
    score_optional?: short = 123;
}

function testWithArgs(a: short): Object {
    return testWithArgsNative(a);
}

function testPostArray(arr: short[]): void {
    return testPostArrayNative(arr);
}
