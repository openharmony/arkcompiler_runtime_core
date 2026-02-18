/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
    Mandatory call to another same-class constructor that uses the keyword this.
    Mandatory call to a superconstructor if a class has an extension clause.
---*/

class Point {
    x: number = -1.0;
    y: number = -1.0;
    constructor(x: number, y: number) {
        this.x = x;
        this.y = y;
    }
}

class ColoredPoint extends Point {
    static readonly WHITE = 0;
    static readonly BLACK = 1;
    color: number;
    // primary constructor:
    constructor(x: number, y: number, color: number) {
        super(x, y); // calls base class constructor as class has 'extends'
        this.color = color;
    }
    // secondary constructor:
    constructor(color: number) {
        this(0, 0, color);
    }
}

function main() {
    let black = new ColoredPoint(ColoredPoint.BLACK);
    arktest.assertTrue(black instanceof ColoredPoint);
    arktest.assertEQ(black.color, 1);
    arktest.assertEQ(black.x, 0.0);
    arktest.assertEQ(black.y, 0.0);

    let white = new ColoredPoint(ColoredPoint.WHITE);
    arktest.assertTrue(white instanceof ColoredPoint);
    arktest.assertEQ(white.color, 0);
    arktest.assertEQ(white.x, 0.0);
    arktest.assertEQ(white.y, 0.0);

    let red = new ColoredPoint(1, 2, 3);
    arktest.assertTrue(red instanceof ColoredPoint);
    arktest.assertEQ(red.color, 3);
    arktest.assertEQ(red.x, 1.0);
    arktest.assertEQ(red.y, 2.0);
}
