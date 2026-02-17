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
    Union type narrowing with interface types and as casts (success path).
    Tests that union of interface types can be narrowed via instanceof
    and cast to access type-specific members at runtime.
    ClassCastError failure path is covered in union_type_cast_narrowing_n.ets.
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

class Triangle implements Drawable {
    override draw(): string { return "triangle"; }
}

function processShape(shape: Shape | Drawable): string {
    if (shape instanceof Circle) {
        let c = shape as Circle;
        return c.draw();
    }
    if (shape instanceof Square) {
        return "square";
    }
    return "unknown";
}

function processArray(shapes: (Shape | Drawable)[]): string {
    let result = "";
    for (let shape of shapes) {
        if (shape instanceof Circle) {
            result += (shape as Circle).draw() + ",";
        } else if (shape instanceof Square) {
            result += "square,";
        }
    }
    return result;
}

function main(): void {
    let circle: Shape | Drawable = new Circle(5.0);
    let square: Shape | Drawable = new Square(3.0);

    arktest.assertEQ(processShape(circle), "circle");
    arktest.assertEQ(processShape(square), "square");

    arktest.assertEQ((circle as Shape).area(), 78.5);
    arktest.assertEQ((circle as Drawable).draw(), "circle");
    arktest.assertEQ((square as Shape).area(), 9.0);

    let shapes: (Shape | Drawable)[] = [new Circle(1.0), new Square(2.0), new Circle(3.0)];
    arktest.assertEQ(processArray(shapes), "circle,square,circle,");

    // Triangle implements only Drawable — exercises the "unknown" branch of processShape
    let triangle: Shape | Drawable = new Triangle();
    arktest.assertEQ(processShape(triangle), "unknown");
    arktest.assertEQ((triangle as Drawable).draw(), "triangle");
}
