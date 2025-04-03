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
desc: Class multi-level inheritance verification.
---*/

class Vehicle {
    wheels: number;
    constructor(wheels: number) {
        this.wheels = wheels;
    }
}

class Car extends Vehicle {
    constructor() {
        super(4);
    }
}

class Tesla extends Car {
    electric: boolean;
}

function main(): void {
    const tesla = new Tesla();
    assertEQ(tesla.wheels, 4);
    assertEQ(tesla instanceof Tesla, true);
    assertEQ(tesla instanceof Car, true);
    assertEQ(tesla instanceof Vehicle, true);
}