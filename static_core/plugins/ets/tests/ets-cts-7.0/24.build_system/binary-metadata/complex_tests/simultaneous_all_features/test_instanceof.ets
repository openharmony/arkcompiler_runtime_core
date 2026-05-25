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
    Verify instanceof type narrowing with cross-module types.
    Imports Shape from lib_types and Circle (extends Shape) from lib_impls.
files: [lib_types.ets]
arktsconfig: arktsconfig-types-only.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "circle:Circle:circle-5"
---*/

import { Shape } from '@panda/types/lib_types'
import { Circle } from '@panda/types/lib_impls'

function main(): void {
    let shape: Shape = new Circle(5.0)
    if (shape instanceof Circle) {
        let c = shape as Circle
        console.log("circle:" + c.describe())
    } else {
        console.log("unknown")
    }
}
