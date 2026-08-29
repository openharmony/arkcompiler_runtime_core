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
  TEST-UNIT-09-0003 GAP-09-0003 GRC-09-0003 CI-09-01-002-02: Abstract
  method must be implemented in concrete subclass
tags: [compile-only]
---*/

abstract class Shape {
    abstract getArea(): number
}

class Circle extends Shape {
    public radius: number

    constructor(radius: number) {
        super()
        this.radius = radius
    }

    getArea(): number {
        return 3.14159 * this.radius * this.radius
    }
}

function main(): void {
    let c = new Circle(5.0)
    console.log(c.getArea())
}
