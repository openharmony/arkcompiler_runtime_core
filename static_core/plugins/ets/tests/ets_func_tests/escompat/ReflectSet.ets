/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

function main(): int {
    let failures: int = 0;

    // set
    failures += test(reflectSetClass(), "Reflect.set on class type");
    failures += test(reflectSetArray(), "Reflect.set on array type");
    failures += test(reflectSetTypeConvert(), "Reflect.set convert type");

    return test(failures, "All tests run");
}

class Point2D {
    static axisnum: number = 2
    x: number
    y: number
    constructor (x_: number, y_: number) {
        this.x = x_
        this.y = y_
    }
}

class Point3D extends Point2D {
    static axisnum: number = 3
    z: number
    constructor (x_: number, y_: number, z_: number) {
        super(x_, y_)
        this.z = z_
    }
}

class C {
    public p1:number = 0;
    public p2:float = 0.0;
    public p3:long = 0;
    public p4:int = 0;
    public p5:short = 0;
}

function arraysAreEqual(a: FixedArray<string>, b: FixedArray<string>): boolean {
    let alen = a.length
    if (alen != b.length) {
        return false
    }
    for (let i = 0; i < alen; i++) {
        if (a[i] != b[i]) {
            return false
        }
    }
    return true
}

function reflectSetClass(): int {
    let result: int = 0

    let p: Point2D = new Point3D(10, 20, 30)

    result += (Reflect.set(p, "x", 40 as number) == true) ? 0 : 1
    result += (Reflect.set(p, "y", 50 as number) == true) ? 0 : 1
    result += (Reflect.set(p, "z", 60 as number) == true) ? 0 : 1

    result += (Reflect.get(p, "x") as Number == 40) ? 0 : 1
    result += (Reflect.get(p, "y") as Number == 50) ? 0 : 1
    result += (Reflect.get(p, "z") as Number == 60) ? 0 : 1

    result += (Reflect.set(p, "axisnum", 10 as number) == false) ? 0 : 1
    result += (Reflect.set(p, "x", "string") == false) ? 0 : 1

    return result
}

function reflectSetTypeConvert(): int {
    let result: int = 0

    let sample = new C();

    let a: byte = 1;
    let b: short = 2;
    let c: int  = 3;
    let d: long = 4;
    let e: float = 5.0;

    Reflect.set(sample, 'p1', a);
    result += (sample.p1 == a) ? 0  : 1
    Reflect.set(sample, 'p1', b);
    result += (sample.p1 == b) ? 0  : 1
    Reflect.set(sample, 'p1', c);
    result += (sample.p1 == c) ? 0  : 1
    Reflect.set(sample, 'p1', d);
    result += (sample.p1 == d) ? 0  : 1
    Reflect.set(sample, 'p1', e);
    result += (sample.p1 == e) ? 0  : 1

    Reflect.set(sample, 'p2', a);
    result += (sample.p2 == a) ? 0  : 1
    Reflect.set(sample, 'p2', b);
    result += (sample.p2 == b) ? 0  : 1
    Reflect.set(sample, 'p2', c);
    result += (sample.p2 == c) ? 0  : 1
    Reflect.set(sample, 'p2', d);
    result += (sample.p2 == d) ? 0  : 1

    Reflect.set(sample, 'p3', a);
    result += (sample.p3 == a) ? 0  : 1
    Reflect.set(sample, 'p3', b);
    result += (sample.p3 == b) ? 0  : 1
    Reflect.set(sample, 'p3', c);
    result += (sample.p3 == c) ? 0  : 1

    Reflect.set(sample, 'p4', a);
    result += (sample.p4 == a) ? 0  : 1
    Reflect.set(sample, 'p4', b);
    result += (sample.p4 == b) ? 0  : 1

    Reflect.set(sample, 'p5', a);
    result += (sample.p5 == a) ? 0  : 1

    return result
}

function reflectSetArray(): int {
    let result: int = 0

    let arr: FixedArray<number> = [10, 20, 30]
    let brr: FixedArray<string> = ["p", "q", "t", "w"]

    result += (Reflect.set(arr, 0, 40 as number) == true) ? 0 : 1
    result += (Reflect.set(arr, 1, 50 as number) == true) ? 0 : 1
    result += (Reflect.set(arr, 2, 60 as number) == true) ? 0 : 1

    result += (Reflect.get(arr, 0) as Number == 40) ? 0 : 1
    result += (Reflect.get(arr, 1) as Number == 50) ? 0 : 1
    result += (Reflect.get(arr, 2) as Number == 60) ? 0 : 1

    result += (Reflect.set(arr, 100, 10 as number) == false) ? 0 : 1
    result += (Reflect.set(arr, 0, "string") == false) ? 0 : 1

    return result
}

function test(result: int, message: String ): int {
    if (result == 0) {
      console.log('PASSED: ' + message);
      return 0;
    }
    console.log('FAILED: ' + message);
    return 1;
}
