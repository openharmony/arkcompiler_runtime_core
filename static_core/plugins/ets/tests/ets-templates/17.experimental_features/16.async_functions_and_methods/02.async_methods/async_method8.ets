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
      Asynchronous method delayed return.
---*/

class MethodWithDelay {
    async methodWithDelay(): Promise<string> {
        return new Promise<string>((resolve: (p: string) => void) => {
            setTimeout(() => {
                resolve(`Resolved after 100 ms`);
            }, 100);
        });
    }
}

async function startTest() {
    const example = new MethodWithDelay();
    const result = await example.methodWithDelay();
    assertEQ(result, "Resolved after 100 ms")
}

function main(): void {
    startTest();
}