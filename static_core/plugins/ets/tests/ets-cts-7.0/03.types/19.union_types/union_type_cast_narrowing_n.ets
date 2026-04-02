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
    Casting a union element to an interface it does not implement throws ClassCastError.
assert: >-
    The result of an as expression is the result of the evaluated expr
    if the actual type of expr is a subtype of target (see Subtyping).
    Otherwise, ClassCastError is thrown.
---*/

interface Shape {
    area(): double
}

interface Drawable {
    draw(): string
}

interface Resizable {
    resize(factor: double): void
}

class Circle implements Shape, Drawable {
    r: double;
    constructor(r: double) { this.r = r; }
    override area(): double { return 3.14 * this.r * this.r; }
    override draw(): string { return "circle"; }
}

class Square implements Shape {
    s: double;
    constructor(s: double) { this.s = s; }
    override area(): double { return this.s * this.s; }
}

function main(): void {
    // Case 1: static type Shape | Drawable, actual type Square — Square does not implement Drawable
    let square: Shape | Drawable = new Square(3.0);
    let caught1 = false;
    try {
        let unreachable = square as Drawable;
    } catch (e) {
        caught1 = true;
        arktest.assertTrue(e instanceof ClassCastError);
    }
    arktest.assertTrue(caught1);

    // Case 2: static type Shape, actual type Circle — Circle does not implement Resizable
    let circle: Shape = new Circle(5.0);
    let caught2 = false;
    try {
        let unreachable = circle as Resizable;
    } catch (e) {
        caught2 = true;
        arktest.assertTrue(e instanceof ClassCastError);
    }
    arktest.assertTrue(caught2);
}
