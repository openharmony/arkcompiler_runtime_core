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

    // get
    failures += test(reflectGetClass(), "Reflect.get on class type");
    failures += test(reflectGetArray(), "Reflect.get on array type");
    failures += test(reflectGetLambda(), "Reflect.get on function type");

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

function reflectGetClass(): int {
    let result: int = 0

    let p: Point2D = new Point3D(10, 20, 30)

    result += (Reflect.get(p, "x") as Number == 10) ? 0 : 1
    result += (Reflect.get(p, "y") as Number == 20) ? 0 : 1
    result += (Reflect.get(p, "z") as Number == 30) ? 0 : 1

    result += (Reflect.get(p, "axisnum") == undefined) ? 0 : 1
    result += (Reflect.get(p, "qwerty") == undefined) ? 0 : 1
    result += (Reflect.get(p, 1) == null) ? 0 : 1

    return result
}

function reflectGetArray(): int {
    let result: int = 0

    let arr: FixedArray<number> = [10, 20, 30]
    let brr: FixedArray<string> = ["p", "q", "t", "w"]

    result += (Reflect.get(arr, 0) as Number == 10) ? 0 : 1
    result += (Reflect.get(arr, 1) as Number == 20) ? 0 : 1
    result += (Reflect.get(arr, 2) as Number == 30) ? 0 : 1

    result += (Reflect.get(brr, 0) == "p") ? 0 : 1
    result += (Reflect.get(brr, 1) == "q") ? 0 : 1
    result += (Reflect.get(brr, 2) == "t") ? 0 : 1

    result += (Reflect.get(arr, "length") as Number == 3) ? 0 : 1
    result += (Reflect.get(brr, "length") as Number == 4) ? 0 : 1

    result += (Reflect.get(arr, "qwerty") == undefined) ? 0 : 1
    result += (Reflect.get(arr, 10) == undefined) ? 0 : 1
    result += (Reflect.get(brr, 4) == undefined) ? 0 : 1

    return result
}

function reflectGetLambda(): int {
    let result: int = 0

    let lambda: (a: number) => number = (a: number): number => {
        return a + 1
    }

    result += (Reflect.get(lambda, 0) == null) ? 0 : 1
    result += (Reflect.get(lambda, "length") as Number == 1) ? 0 : 1
    result += (Reflect.get(lambda, "name") == "") ? 0 : 1

    result += (Reflect.get(lambda, "qwerty") == undefined) ? 0 : 1

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
