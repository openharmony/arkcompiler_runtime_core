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
desc: Package @panda/types - implementations (simultaneous with lib_types)
tags: [not-a-test]
name: lib_impls
---*/

import { Shape, Describable, GenBox } from './lib_types'

// --- Concrete class extending abstract ---
export class Circle extends Shape implements Describable {
    private _radius: number
    constructor(radius: number) {
        super()
        this._radius = radius
    }
    override area(): number {
        return 3.14159 * this._radius * this._radius
    }
    override perimeter(): number {
        return 2.0 * 3.14159 * this._radius
    }
    display(): string {
        return "Circle"
    }
    getName(): string {
        return "circle-" + this._radius.toString()
    }
    describe(): string {
        return this.display() + ":" + this.getName()
    }
}

// --- Class extending generic ---
export class NumBox extends GenBox<number> {
    constructor(value: number) {
        super(value)
    }
    doubled(): number {
        return this.value * 2
    }
}

