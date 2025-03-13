/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
desc: 2.9.7 Null Literal, 3.8.12 Nullish Types
name: nullable-method-default-param_0.ets

---*/

class TestC {
    testMethod(required: String, param: (Int | null) = 100): void {
        assertTrue( param === null)
    }
}



function main(): void {
    new TestC().testMethod("Hello World", null);

}
