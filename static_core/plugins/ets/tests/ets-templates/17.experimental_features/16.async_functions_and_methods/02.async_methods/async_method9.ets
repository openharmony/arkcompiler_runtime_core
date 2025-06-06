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
      chain call asynchronous method.
---*/

class MethodChaining {
    async methodReturnString(): Promise<string> {
        return "Hello";
    }

    async methodReturnNumber(): Promise<number> {
        return 42.0;
    }

    async methodChaining(): Promise<string> {
        const result1 = await this.methodReturnString();
        const result2 = await this.methodReturnNumber();
        return `${result1} ${result2}`;
    }
}

async function startTest() {
    const example = new MethodChaining();
    const result = await example.methodChaining();
    assertEQ(result, "Hello 42")
}

function main(): void {
    startTest();
}