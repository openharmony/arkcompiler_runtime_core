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
desc: Test cases for various usage scenarios of asynchronous function.
      Using async/await nesting in asynchronous functions.
---*/

async function methodNestedAsync(): Promise<string> {
    const result = await (async (): Promise<string> => {
         return "Nested async!";
    })();
    return result;
}

async function startTest() {
    const result = await methodNestedAsync();
    assertEQ(result, "Nested async!");
}

function main(): void {
    startTest();
}