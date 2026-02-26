/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: The method with the same name has different parameter types and quantities, but the return type is the same.
---*/

let res: int = 0

interface Parent1 {
    log(message: string): void;
}

interface Parent2 {
    log(message: string, level: number): void;
}

interface Child extends Parent1, Parent2 {}

//The signature is override-compatible only if number of type parameters of either method is the same
class ChildClass implements Child {
    override log(message: string): void {
        res = 1
    }

    override log(message: string, level: number): void {
        res = 2
    }
}

function main() {
  let c: ChildClass = new ChildClass()
  c.log("str")
  arktest.assertEQ(res, 1)
  c.log("str", 2)
  arktest.assertEQ(res, 2)
}