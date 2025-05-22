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
desc: 05 Generics
name: 05.generics/genericTypeArgumentWithDefaultParameterValue_06
tags: []
---*/

class UserType {
    name: string
}

interface I<T = UserType> {
    foo(p: T) : T
}

class A : implements I {
    data: T
    constructor(p: T) {
        this.data = p.type
    }

    foo(p : T) : T {
        return data;
    }
}


function main(): void {
    let a1 = new A();
    let a2 = new A<UserType>();
    let a3 = new A<string>;

    let ra1 = a1.foo(new UserType());
    let ra2 = a2.foo(new UserType());
    let ra3 = a3.foo("hello")

    assertTrue( ra1 instanceof UserType)
    assertTrue( ra2 instanceof UserType)
    assertTrue( ra3 instanceof string)
}

