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
desc: Abstract class export
tags: [not-a-test, compile-only]
name: test_lib
arktsconfig: arktsconfig-lib.json
es2panda_options: ['emit-declaration']
---*/

export abstract class Shape {
    name: string = ""
    abstract area(): double
    describe(): string {
        return this.name + ": " + this.area()
    }
}

export class Rectangle extends Shape {
    w: double
    h: double
    constructor(w: double, h: double) {
        super()
        this.name = "Rect"
        this.w = w
        this.h = h
    }
    area(): double {
        return this.w * this.h
    }
}
