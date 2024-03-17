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
    // has
    failures += test(reflectHas(), "Reflect.has");

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
    public norm(): number {
        return this.x + this.y
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

function reflectHas(): int {
    let result: int = 0

    let arr: number[] = [10, 20, 30]
    let str: string = "abc"

    let cl: Point2D = new Point3D(10, 20, 30)

    let lambda: (a: number) => number = (a: number): number => {
        return a + 1
    }

    result += (Reflect.has(arr, 0) == true) ? 0 : 1
    result += (Reflect.has(arr, 3) == false) ? 0 : 1
    result += (Reflect.has(arr, "length") == true) ? 0 : 1
    result += (Reflect.has(arr, "qwerty") == false) ? 0 : 1
    result += (Reflect.has(arr, "toString") == true) ? 0 : 1
    result += (Reflect.has(arr, "toLocaleString") == true) ? 0 : 1
    result += (Reflect.has(arr, "keys") == false) ? 0 : 1

    result += (Reflect.has(str, 0) == true) ? 0 : 1
    result += (Reflect.has(str, 3) == false) ? 0 : 1
    result += (Reflect.has(str, "length") == true) ? 0 : 1
    result += (Reflect.has(str, "qwerty") == false) ? 0 : 1
    result += (Reflect.has(str, "toString") == true) ? 0 : 1
    result += (Reflect.has(str, "hasOwnProperty") == true) ? 0 : 1
    result += (Reflect.has(str, "entries") == false) ? 0 : 1

    result += (Reflect.has(cl, "x") == true) ? 0 : 1
    result += (Reflect.has(cl, "y") == true) ? 0 : 1
    result += (Reflect.has(cl, "z") == true) ? 0 : 1
    result += (Reflect.has(cl, "axisnum") == false) ? 0 : 1
    result += (Reflect.has(cl, "asdasd") == false) ? 0 : 1
    result += (Reflect.has(cl, 0) == false) ? 0 : 1
    result += (Reflect.has(cl, "norm") == true) ? 0 : 1
    result += (Reflect.has(cl, "toString") == true) ? 0 : 1
    result += (Reflect.has(cl, "hasOwnProperty") == true) ? 0 : 1
    result += (Reflect.has(cl, "getOwnPropertyNames") == false) ? 0 : 1

    result += (Reflect.has(lambda, "length") == true) ? 0 : 1
    result += (Reflect.has(lambda, "name") == true) ? 0 : 1
    result += (Reflect.has(lambda, 0) == false) ? 0 : 1
    result += (Reflect.has(lambda, "toString") == true) ? 0 : 1
    result += (Reflect.has(lambda, "toLocaleString") == true) ? 0 : 1
    result += (Reflect.has(lambda, "hasOwnProperty") == true) ? 0 : 1

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
