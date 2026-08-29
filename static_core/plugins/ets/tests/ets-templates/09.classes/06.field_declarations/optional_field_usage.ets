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
desc: "TEST-UNIT-09-0022 GAP-09-0022 GRC-09-0022 CI-09-06-003-01: Optional fields can be undefined"
tags: [compile-only]
---*/

class Person {
    public name: string
    public age?: number  // Optional field

    constructor(name: string, age?: number) {
        this.name = name
        this.age = age
    }
}

function main(): void {
    let p1 = new Person('Alice', 30)
    let p2 = new Person('Bob')
    console.log(p1.age)  // 30
    console.log(p2.age)  // undefined
}
