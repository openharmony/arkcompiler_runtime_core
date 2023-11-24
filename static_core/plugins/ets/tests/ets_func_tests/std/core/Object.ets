/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

    failures += test(objectKeys(), "Object.keys");

    failures += test(objectValues(), "Object.values");

    failures += test(objectEntries(), "Object.entries");

    failures += test(objectGetOwnPropertyNames(), "Object.getOwnPropertyNames");

    failures += test(objectHasOwnProperty(), "Object.hasOwnProperty");

    return test(failures, "All tests run");
}

type Tuple = Object[]

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

function strArraysAreEqual(a: string[], b: string[]): boolean {
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

function objArraysAreEqual(a: Object[], b: Object[]): boolean {
    let alen = a.length
    if (alen != b.length) {
        return false
    }
    for (let i = 0; i < alen; i++) {
        if (!(a[i].equals(b[i]))) {
            return false
        }
    }
    return true
}

function tupleArraysAreEqual(a: Tuple[], b: Tuple[]): boolean {
    let alen = a.length
    if (alen != b.length) {
        return false
    }
    for (let i = 0; i < alen; i++) {
        if (a[i].length != b[i].length) {
            return false
        }
        let aalen = a[i].length
        for (let j = 0; j < aalen; j++) {
            if (!(a[i][j].equals(b[i][j]))) {
                return false
            }
        }
    }
    return true
}

function objectKeys(): int {
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

    result += strArraysAreEqual(Object.keys(c), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.keys(bo), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.keys(bt), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.keys(sh), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.keys(i), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.keys(lo), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.keys(fl), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.keys(dou), emptyArr) ? 0 : 1

    result += strArraysAreEqual(Object.keys(arr), ["0", "1", "2"]) ? 0 : 1
    result += strArraysAreEqual(Object.keys(str), ["0", "1", "2"]) ? 0 : 1

    result += strArraysAreEqual(Object.keys(cl), ["x", "y", "z"]) ? 0 : 1

    result += strArraysAreEqual(Object.keys(lambda), emptyArr) ? 0 : 1

    return result
}

function objectValues(): int {
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

    let emptyArr: Object[] = []

    result += objArraysAreEqual(Object.values__(c), emptyArr) ? 0 : 1
    result += objArraysAreEqual(Object.values__(bo), emptyArr) ? 0 : 1
    result += objArraysAreEqual(Object.values__(bt), emptyArr) ? 0 : 1
    result += objArraysAreEqual(Object.values__(sh), emptyArr) ? 0 : 1
    result += objArraysAreEqual(Object.values__(i), emptyArr) ? 0 : 1
    result += objArraysAreEqual(Object.values__(lo), emptyArr) ? 0 : 1
    result += objArraysAreEqual(Object.values__(fl), emptyArr) ? 0 : 1
    result += objArraysAreEqual(Object.values__(dou), emptyArr) ? 0 : 1

    let arrVal: Object[] = [new Number(10), new Number(20), new Number(30)]
    result += objArraysAreEqual(Object.values__(arr), arrVal) ? 0 : 1

    let strVal: Object[] = ["a" as string, "b" as string, "c" as string]
    result += objArraysAreEqual(Object.values__(str), strVal) ? 0 : 1

    let clVal: Object[] = [new Number(10), new Number(20), new Number(30)]
    result += objArraysAreEqual(Object.values__(cl), clVal) ? 0 : 1

    result += objArraysAreEqual(Object.values__(lambda), emptyArr) ? 0 : 1

    return result
}

function objectEntries(): int {
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

    let emptyTuple: Tuple[] = new Tuple[0]

    result += tupleArraysAreEqual(Object.entries(c), emptyTuple) ? 0 : 1
    result += tupleArraysAreEqual(Object.entries(bo), emptyTuple) ? 0 : 1
    result += tupleArraysAreEqual(Object.entries(bt), emptyTuple) ? 0 : 1
    result += tupleArraysAreEqual(Object.entries(sh), emptyTuple) ? 0 : 1
    result += tupleArraysAreEqual(Object.entries(i), emptyTuple) ? 0 : 1
    result += tupleArraysAreEqual(Object.entries(lo), emptyTuple) ? 0 : 1
    result += tupleArraysAreEqual(Object.entries(fl), emptyTuple) ? 0 : 1
    result += tupleArraysAreEqual(Object.entries(dou), emptyTuple) ? 0 : 1

    let arrVal: Tuple[] = new Tuple[3]
    arrVal[0] = new Object[2]
    arrVal[0] = ["0", new Number(10)]
    arrVal[1] = new Object[2]
    arrVal[1] = ["1", new Number(20)]
    arrVal[2] = new Object[2]
    arrVal[2] = ["2", new Number(30)]
    result += tupleArraysAreEqual(Object.entries(arr), arrVal) ? 0 : 1

    let strVal: Tuple[] = new Tuple[3]
    strVal[0] = new Object[2]
    strVal[0] = ["0", "a"]
    strVal[1] = new Object[2]
    strVal[1] = ["1", "b"]
    strVal[2] = new Object[2]
    strVal[2] = ["2", "c"]
    result += tupleArraysAreEqual(Object.entries(str), strVal) ? 0 : 1

    let clVal: Tuple[] = new Tuple[3]
    clVal[0] = new Object[2]
    clVal[0] = ["x", new Number(10)]
    clVal[1] = new Object[2]
    clVal[1] = ["y", new Number(20)]
    clVal[2] = new Object[2]
    clVal[2] = ["z", new Number(30)]
    result += tupleArraysAreEqual(Object.entries(cl), clVal) ? 0 : 1

    result += tupleArraysAreEqual(Object.entries(lambda), emptyTuple) ? 0 : 1

    return result
}

function objectGetOwnPropertyNames(): int {
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

    result += strArraysAreEqual(Reflect.ownKeys(c), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Reflect.ownKeys(bo), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Reflect.ownKeys(bt), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Reflect.ownKeys(sh), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Reflect.ownKeys(i), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Reflect.ownKeys(lo), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Reflect.ownKeys(fl), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Reflect.ownKeys(dou), emptyArr) ? 0 : 1

    result += strArraysAreEqual(Reflect.ownKeys(arr), ["0", "1", "2", "length"]) ? 0 : 1
    result += strArraysAreEqual(Reflect.ownKeys(str), ["0", "1", "2", "length"]) ? 0 : 1

    result += strArraysAreEqual(Reflect.ownKeys(cl), ["x", "y", "z"]) ? 0 : 1

    result += strArraysAreEqual(Reflect.ownKeys(lambda), ["length", "name"]) ? 0 : 1

    return result
}

function objectHasOwnProperty(): int {
    let result: int = 0

    let arr: number[] = [10, 20, 30]
    let str: string = "abc"

    let cl: Point2D = new Point3D(10, 20, 30)

    let lambda: (a: number) => number = (a: number): number => {
        return a + 1
    }

    result += ((arr as Object).hasOwnProperty(0) == true) ? 0 : 1
    result += ((arr as Object).hasOwnProperty(3) == false) ? 0 : 1
    result += ((arr as Object).hasOwnProperty("length") == true) ? 0 : 1
    result += ((arr as Object).hasOwnProperty("qwerty") == false) ? 0 : 1

    result += (str.hasOwnProperty(0) == true) ? 0 : 1
    result += (str.hasOwnProperty(3) == false) ? 0 : 1
    result += (str.hasOwnProperty("length") == true) ? 0 : 1
    result += (str.hasOwnProperty("qwerty") == false) ? 0 : 1

    result += (cl.hasOwnProperty("x") == true) ? 0 : 1
    result += (cl.hasOwnProperty("y") == true) ? 0 : 1
    result += (cl.hasOwnProperty("z") == true) ? 0 : 1
    result += (cl.hasOwnProperty("axisnum") == false) ? 0 : 1
    result += (cl.hasOwnProperty("asdasd") == false) ? 0 : 1
    result += (cl.hasOwnProperty(0) == false) ? 0 : 1

    result += ((lambda as Object).hasOwnProperty("length") == true) ? 0 : 1
    result += ((lambda as Object).hasOwnProperty("name") == true) ? 0 : 1
    result += ((lambda as Object).hasOwnProperty(0) == false) ? 0 : 1

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
