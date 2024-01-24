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

    // set
    failures += test(reflectSetClass(), "Reflect.set on class type");
    failures += test(reflectSetArray(), "Reflect.set on array type");

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

function reflectSetArray(): int {
    let result: int = 0

    let arr: number[] = [10, 20, 30]
    let brr: string[] = ["p", "q", "t", "w"]

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
