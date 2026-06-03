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
desc: Shape hierarchy for narrowing conversion tests
tags: [not-a-test, compile-only]
name: lib_shapes
arktsconfig: arktsconfig-lib.json
es2panda_options: ['emit-declaration']
---*/

export class Shape {
    kind: string
    constructor(k: string) { this.kind = k }
    describe(): string { return this.kind }
}

export class Circle extends Shape {
    radius: int
    constructor(r: int) {
        super("circle")
        this.radius = r
    }
    override describe(): string { return "circle:r=" + this.radius }
}

export class Square extends Shape {
    side: int
    constructor(s: int) {
        super("square")
        this.side = s
    }
    override describe(): string { return "square:s=" + this.side }
}

export interface Drawable {
    draw(): string
}

export class DrawableCircle extends Circle implements Drawable {
    constructor(r: int) {
        super(r)
    }
    draw(): string { return "draw:" + this.kind }
}
