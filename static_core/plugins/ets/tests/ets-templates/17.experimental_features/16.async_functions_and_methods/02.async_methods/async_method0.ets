/*
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

/*---
desc: Test cases for various usage scenarios of asynchronous method.
      The asynchronous method returns a string.
---*/

class MethodReturnString {
    async methodReturnString(): Promise<string> {
        return "Hello, World!";
    }
}

function startTest() {
    const example = new MethodReturnString();
    const result = await example.methodReturnString();
    assertEQ(result, "Hello, World!");
}

function main(): void {
    startTest();
}