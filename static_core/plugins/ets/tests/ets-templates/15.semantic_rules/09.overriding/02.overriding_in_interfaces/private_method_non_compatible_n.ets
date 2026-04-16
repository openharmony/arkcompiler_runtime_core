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

/*---
desc: >-
    A compile-time error occurs if subinterface tries to access private method 
tags: [compile-only, negative]
expected_out: "Semantic error ESE0139: Signature foo(): undefined is not visible here."
---*/

interface Base {
    method_1()
    method_2(p: number)
    private foo() {}
}

interface Derived extends Base {
    method_1() //overriding
    method_2(p: string) { // overloading
        this.foo(1.0) // OK
        this.foo() // CTE expected, Base.foo() is not accessible here at all
    }
    foo(p: number): void // it is just a new method declaration
}
