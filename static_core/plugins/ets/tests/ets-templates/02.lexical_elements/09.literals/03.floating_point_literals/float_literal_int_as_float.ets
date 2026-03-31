/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Integer literal used where float parameter is expected, with virtual dispatch through interface
---*/

interface Scalable {
    scale(factor: int): void;
}

class Pt2D implements Scalable {
    x: float;
    y: float;

    constructor(x: float, y: float) {
        this.x = x;
        this.y = y;
    }

    scale(factor: int): void {
        this.x = this.x * (factor.toFloat());
        this.y = this.y * (factor.toFloat());
    }
}

class Pt3D extends Pt2D {
    z: float;

    constructor(x: float, y: float, z: float) {
        super(x, y);
        this.z = z;
    }

    override scale(factor: int): void {
        super.scale(factor);
        this.z = this.z * (factor.toFloat());
    }
}

function main(): void {
    // -2 is an integer literal passed where float parameter is expected
    let pt = new Pt3D(0.1f, -2, 3.14f);
    // Verify integer literal -2 was stored.toFloat() -2.0 (the coverage point)
    arktest.assertEQ(pt.y, -2.0f);

    let s: Scalable = pt;
    s.scale(3);
    arktest.assertEQ(pt.x, 0.3f);
    arktest.assertEQ(pt.y, -6.0f);
    arktest.assertEQ(pt.z, 3.14f * 3.0f);

    // Negative float literal with absolute value in [0.1, 1.0): exercises numberLiteral.cpp:316 branch 1
    let fNeg: float = -0.5f;
    arktest.assertEQ(fNeg, -0.5f);

    // Reassign interface to base type; use non-zero y to make assertion meaningful
    let p2 = new Pt2D(2.71f, 1.5f);
    (p2 as Scalable).scale(2);
    arktest.assertEQ(p2.x, 2.71f * 2.0f);
    arktest.assertEQ(p2.y, 3.0f);
}
