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
    Test widening reference conversion across packages.
files: [lib_hierarchy.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "car:Sedan"
        - "truck:Ford"
        - "car:Civic"
---*/

import { Vehicle, Car, Truck, describeVehicle } from '@panda/lib/lib_hierarchy'

let v: Vehicle = new Car("Sedan", 4)
console.log(v.describe())

console.log(describeVehicle(new Truck("Ford")))

let c = new Car("Civic", 2)
let v2: Vehicle = c
console.log(v2.describe())
