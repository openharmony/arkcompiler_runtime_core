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
desc: Abstract class import
files: [test_lib.ets]
name: test_import
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "Circle: 300"
        - "Rect: 12"
---*/

import {Shape, Rectangle} from '@panda/lib/test_lib'

class Circle extends Shape {
    r: double
    constructor(r: double) {
        super()
        this.name = "Circle"
        this.r = r
    }
    area(): double {
        return 3.0 * this.r * this.r
    }
}

let c = new Circle(10.0)
let rect = new Rectangle(3.0, 4.0)
console.log(c.describe())
console.log(rect.describe())
