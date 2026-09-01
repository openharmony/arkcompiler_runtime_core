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
desc: "TEST-UNIT-09-0017 GAP-09-0017 GRC-09-0017 CI-09-10-004-01: Polymorphism through method overriding"
tags: [compile-only]
---*/

abstract class Vehicle {
    abstract move(): string
}

class Car extends Vehicle {
    move(): string {
        return 'Car drives on road'
    }
}

class Boat extends Vehicle {
    move(): string {
        return 'Boat sails on water'
    }
}

function main(): void {
    let vehicles: Vehicle[] = [new Car(), new Boat()]
    for (let v of vehicles) {
        console.log(v.move())  // Dynamic dispatch
    }
}
