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
desc: Class hierarchy for widening conversion tests
tags: [not-a-test, compile-only]
name: lib_hierarchy
arktsconfig: arktsconfig-lib.json
es2panda_options: ['emit-declaration']
---*/

export class Vehicle {
    name: string
    constructor(n: string) { this.name = n }
    describe(): string { return "vehicle:" + this.name }
}

export class Car extends Vehicle {
    doors: int
    constructor(n: string, d: int) {
        super(n)
        this.doors = d
    }
    override describe(): string { return "car:" + this.name }
}

export class Truck extends Vehicle {
    constructor(n: string) {
        super(n)
    }
    override describe(): string { return "truck:" + this.name }
}

export function describeVehicle(v: Vehicle): string {
    return v.describe()
}
