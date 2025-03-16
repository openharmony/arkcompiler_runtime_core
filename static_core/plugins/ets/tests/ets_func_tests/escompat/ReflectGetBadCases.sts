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

    // get
    failures += test(reflectGetBadCases(), "Reflect.get on types without fields and elements");

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

function reflectGetBadCases(): int {
    let c: char = c'c'
    let bo: boolean = true
    let bt: byte = 10
    let sh: short = 20
    let i: int = 30
    let lo: long = 40
    let fl: float = 50.0
    let dou: double = 60.0
    let s: string = "abc"

    let result: int = 0
    try {
        Reflect.get(c, "a")
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must have fields", 0) ? 0 : 1
    }

    try {
        Reflect.get(bo, "a")
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must have fields", 0) ? 0 : 1
    }

    try {
        Reflect.get(bt, "a")
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must have fields", 0) ? 0 : 1
    }

    try {
        Reflect.get(sh, "a")
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must have fields", 0) ? 0 : 1
    }

    try {
        Reflect.get(i, "a")
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must have fields", 0) ? 0 : 1
    }

    try {
        Reflect.get(lo, "a")
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must have fields", 0) ? 0 : 1
    }

    try {
        Reflect.get(fl, "a")
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must have fields", 0) ? 0 : 1
    }

    try {
        Reflect.get(dou, "a")
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must have fields", 0) ? 0 : 1
    }

    try {
        Reflect.get(s, "a")
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must have fields", 0) ? 0 : 1
    }

    // ----------

    try {
        Reflect.get(c, 1)
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must be indexed", 0) ? 0 : 1
    }

    try {
        Reflect.get(bo, 1)
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must be indexed", 0) ? 0 : 1
    }

    try {
        Reflect.get(bt, 1)
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must be indexed", 0) ? 0 : 1
    }

    try {
        Reflect.get(sh, 1)
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must be indexed", 0) ? 0 : 1
    }

    try {
        Reflect.get(i, 1)
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must be indexed", 0) ? 0 : 1
    }

    try {
        Reflect.get(lo, 1)
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must be indexed", 0) ? 0 : 1
    }

    try {
        Reflect.get(fl, 1)
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must be indexed", 0) ? 0 : 1
    }

    try {
        Reflect.get(dou, 1)
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must be indexed", 0) ? 0 : 1
    }

    try {
        Reflect.get(s, 1)
    } catch (e: Error) {
        result += e.toString().contains("`target` argument of Reflect.get must be indexed", 0) ? 0 : 1
    }

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
