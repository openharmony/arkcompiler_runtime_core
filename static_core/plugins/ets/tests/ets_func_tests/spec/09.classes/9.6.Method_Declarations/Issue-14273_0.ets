/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: Issue 14273 test
name: Issue-14273_0.ets
tags: []

---*/

class Base {
    public field: number = 20;
    public static field: number = 10;
}

function main(): void {
    let a = new Base();

    assertEQ( a.field, 20)
    assertEQ( Base.field, 10)

}
