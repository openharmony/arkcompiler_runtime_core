// Copyright (c) 2026 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

class Point {
    constructor(x, y) {
        this.x = x;
        this.y = y;
    }

    get sum() {
        return this.x + this.y;
    }

    set sum(v) {
        this.x = v;
        this.y = 0;
    }

    move(dx, dy) {
        this.x += dx;
        this.y += dy;
        return this;
    }

    scale(k) {
        this.x *= k;
        this.y *= k;
        return this;
    }

    dist() {
        return Math.sqrt(this.x * this.x + this.y * this.y);
    }

    static origin() {
        return new Point(0, 0);
    }

    static fromArray(arr) {
        return new Point(arr[0], arr[1]);
    }
}

class Rect {
    constructor(w, h) {
        this.w = w;
        this.h = h;
    }

    get area() {
        return this.w * this.h;
    }

    grow(dw, dh) {
        this.w += dw;
        this.h += dh;
        return this;
    }
}

function makePoints() {
    let a = Point.origin().move(1, 2).scale(3);
    let b = Point.fromArray([4, 5]);
    a.sum = 9;
    return [a.sum, a.dist(), b.x, b.y];
}

let p = makePoints();
let r = new Rect(3, 4).grow(1, 1);
console.log(p[0], p[1], r.area);
