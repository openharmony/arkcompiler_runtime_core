/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

    // ownKeys
    failures += test(reflectOwnKeys(), "Reflect.ownKeys");

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

function arraysAreEqual(a: string[], b: string[]): boolean {
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

function reflectOwnKeys(): int {
    let result: int = 0

    let c: char = c'c'
    let bo: boolean = true
    let bt: byte = 10
    let sh: short = 20
    let i: int = 30
    let lo: long = 40
    let fl: float = 50.0
    let dou: double = 60.0

    let arr: number[] = [10, 20, 30]
    let str: string = "abc"

    let cl: Point2D = new Point3D(10, 20, 30)

    let lambda: (a: number) => number = (a: number): number => {
        return a + 1
    }

    let emptyArr: string[] = []

    result += arraysAreEqual(Reflect.ownKeys(c), emptyArr) ? 0 : 1
    result += arraysAreEqual(Reflect.ownKeys(bo), emptyArr) ? 0 : 1
    result += arraysAreEqual(Reflect.ownKeys(bt), emptyArr) ? 0 : 1
    result += arraysAreEqual(Reflect.ownKeys(sh), emptyArr) ? 0 : 1
    result += arraysAreEqual(Reflect.ownKeys(i), emptyArr) ? 0 : 1
    result += arraysAreEqual(Reflect.ownKeys(lo), emptyArr) ? 0 : 1
    result += arraysAreEqual(Reflect.ownKeys(fl), emptyArr) ? 0 : 1
    result += arraysAreEqual(Reflect.ownKeys(dou), emptyArr) ? 0 : 1

    result += arraysAreEqual(Reflect.ownKeys(arr), ["0", "1", "2", "length"]) ? 0 : 1
    result += arraysAreEqual(Reflect.ownKeys(str), ["0", "1", "2", "length"]) ? 0 : 1

    result += arraysAreEqual(Reflect.ownKeys(cl), ["x", "y", "z"]) ? 0 : 1

    result += arraysAreEqual(Reflect.ownKeys(lambda), ["length", "name"]) ? 0 : 1

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
