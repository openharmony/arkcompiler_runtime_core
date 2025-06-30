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

const emptyArray: string[] = []

function compareArray(first: string[], second: string[]) {
    arktest.assertEQ(first.length, second.length)
    for (let i = 0; i < first.length; i++) {
        arktest.assertEQ(first[i], second[i])
    }
}

function booleanTest() {
    compareArray(Reflect.ownKeys(true), emptyArray)
    compareArray(Reflect.ownKeys(false), emptyArray)
}

function byteTest() {
    compareArray(Reflect.ownKeys(0 as byte), emptyArray)
    compareArray(Reflect.ownKeys(Byte.MIN_VALUE), emptyArray)
    compareArray(Reflect.ownKeys(Byte.MAX_VALUE), emptyArray)
}

function charTest() {
    compareArray(Reflect.ownKeys(c'\0'), emptyArray)
    compareArray(Reflect.ownKeys(c'1'), emptyArray)
}

function shortTest() {
    compareArray(Reflect.ownKeys(0 as short), emptyArray)
    compareArray(Reflect.ownKeys(Short.MIN_VALUE), emptyArray)
    compareArray(Reflect.ownKeys(Short.MAX_VALUE), emptyArray)
}

function intTest() {
    compareArray(Reflect.ownKeys(0), emptyArray)
    compareArray(Reflect.ownKeys(Int.MIN_VALUE), emptyArray)
    compareArray(Reflect.ownKeys(Int.MAX_VALUE), emptyArray)
}

function longTest() {
    compareArray(Reflect.ownKeys(0 as long), emptyArray)
    compareArray(Reflect.ownKeys(Long.MIN_VALUE), emptyArray)
    compareArray(Reflect.ownKeys(Long.MAX_VALUE), emptyArray)
}

function floatTest() {
    compareArray(Reflect.ownKeys(Double.toFloat(0.0)), emptyArray)
    compareArray(Reflect.ownKeys(Float.MIN_VALUE), emptyArray)
    compareArray(Reflect.ownKeys(Float.MAX_VALUE), emptyArray)
}

function doubleTest() {
    compareArray(Reflect.ownKeys(0.0), emptyArray)
    compareArray(Reflect.ownKeys(Double.MIN_VALUE), emptyArray)
    compareArray(Reflect.ownKeys(Double.MAX_VALUE), emptyArray)
}

function stringTest() {
    let str = ''
    compareArray(Reflect.ownKeys(str), ['length'])
    str = 'abc'
    compareArray(Reflect.ownKeys(str), ['0', '1', '2', 'length'])
}

class Point2D {
    public static axisnum: number = 2
    public x: number
    public y: number
    constructor (x_: number, y_: number) {
        this.x = x_
        this.y = y_
    }
}

class Point3D extends Point2D {
    public static axisnum: number = 3
    public z: number
    constructor (x_: number, y_: number, z_: number) {
        super(x_, y_)
        this.z = z_
    }
}

function classTest() {
    let p1 = new Point2D(1, 2)
    compareArray(Reflect.ownKeys(p1), ['x', 'y'])
    let p2 = new Point3D(3, 4, 5)
    compareArray(Reflect.ownKeys(p2), ['x', 'y', 'z'])
}

function fixedArrayTest() {
    let arr: FixedArray<int> = []
    compareArray(Reflect.ownKeys(arr), ['length'])
    arr = [1, 20, 30]
    compareArray(Reflect.ownKeys(arr), ['0', '1', '2', 'length'])

    let arr2: FixedArray<Point2D> = [new Point2D(7, 9), new Point3D(0, 0, 1)]
    compareArray(Reflect.ownKeys(arr2), ['0', '1', 'length'])
}

function arrayTest() {
    let arr: Array<number> = []
    compareArray(Reflect.ownKeys(arr), ['length'])
    arr = [1.0, 12.5, 5.0]
    compareArray(Reflect.ownKeys(arr), ['0', '1', '2', 'length'])

    let arr2: Array<Point2D> = [new Point2D(0, 1), new Point3D(0, 1, 2)]
    compareArray(Reflect.ownKeys(arr2), ['0', '1', 'length'])

    let arr3 = new Array<int>()
    compareArray(Reflect.ownKeys(arr3), ['length'])

    let arr4 = new Array<int>(5)
    compareArray(Reflect.ownKeys(arr4), ['0', '1', '2', '3', '4', 'length'])
}

function lambdaTest() {
    let lambda: (a: number) => number = (a: number): number => {
        return a + 1
    }
    compareArray(Reflect.ownKeys(lambda), ["length", "name"])

    compareArray(Reflect.ownKeys(arrayTest), ["length", "name"])
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Reflect.ownKeys tests')
    suite.addTest('for boolean', booleanTest)
    suite.addTest('for byte', byteTest)
    suite.addTest('for char', charTest)
    suite.addTest('for short', shortTest)
    suite.addTest('for int', intTest)
    suite.addTest('for long', longTest)
    suite.addTest('for float', floatTest)
    suite.addTest('for double', doubleTest)
    suite.addTest('for string', stringTest)
    suite.addTest('for class', classTest)
    suite.addTest('for FixedArray', fixedArrayTest)
    suite.addTest('for Array', arrayTest)
    suite.addTest('for lambda', lambdaTest)
    return suite.run()
}
