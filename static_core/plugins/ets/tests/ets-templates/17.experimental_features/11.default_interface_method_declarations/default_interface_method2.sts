/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
    A block of code that represents the body of a default method in
    an interface provides a default implementation for any
    class unless such class overrides the method to implement the interface.
---*/

interface Vehicle {
    getHorsePower(rpm: int, torque: int): int {
        return (rpm * torque) / 5;
    }
}

interface Auto extends Vehicle {}

class A implements Auto {
    // default interface method overridden
    public getHorsePower(rpm: int, torque: int): int {
        return rpm * torque;
    }
}

class B extends A {}

class C implements Auto {}

function main(): void {
    let a: A = new A()
    assertEQ( a.getHorsePower(5, 5), 25 )

    let b: B = new B()
    assertEQ( b.getHorsePower(5, 5), 25 )

    let c: C = new C()
    assertEQ( c.getHorsePower(5, 5), 5 )
}
