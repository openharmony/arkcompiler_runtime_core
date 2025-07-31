/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

const RESULT_SUCCESS = 0
const RESULT_FAILURE = 1

class Point {
    static s = "s"

    x: number
    y: number

    constructor() {
        this(0, 0)
    }

    constructor(x: number, y: number) {
        this.x = x
        this.y = y
    }

    toString(): string {
        return `Point(x=${this.x},y=${this.y})`
    }
}

class Trash {
    point: Nullish<Point>

    constructor() {
        this.point = undefined
    }

    constructor(point: Point) {
        this.point = point
    }
}

function newUndefiningReviver(...keys: FixedArray<string>): (k: string, v: NullishType) => NullishType {
    return (key: string, val: NullishType) => Array.from<string>(keys).indexOf(key) != -1 ? undefined : val
}

function newNullingReviver(...keys: FixedArray<string>): (k: string, v: NullishType) => NullishType {
    return (key: string, val: NullishType) => Array.from<string>(keys).indexOf(key) != -1 ? null : val
}

function newReplacingReviver(replacements: Map<string, NullishType>): (k: string, v: NullishType) => NullishType {
    return (key: string, val: NullishType) => {
        if (replacements.has(key)) {
            return replacements.get(key)
        }

        return val
    }
}

function replacingReviver(key: string, val: NullishType): NullishType {
    if (key == "x") {
        return 10.0
    } else {
        return val
    }
}

const POINT_TYPE = typeFor([] as FixedArray<Point>)

function jsonParseUsingUndefinedReviver(): int {
    const point = new Point(1, 2)

    const parsedPoint: Point = JSON.parse<Point>(JSON.stringify(point), undefined, POINT_TYPE)!

    arktest.assertEQ(point.x, parsedPoint.x)
    arktest.assertEQ(point.y, parsedPoint.y)
    
    return RESULT_SUCCESS
}

function jsonParseUsingRevivierWhichReturnsUndefined(): int {
    const point = new Point(1, 2)

    const parsedPoint = JSON.parse<Point>(JSON.stringify(point), newUndefiningReviver(""), POINT_TYPE)

    arktest.assertEQ(undefined, parsedPoint)

    return RESULT_SUCCESS
}

const STRING_ARRAY_TYPE = Type.of([] as FixedArray<string>)

function jsonParseArrayUsingReviverWhichReturnsUndefined(): int {
    const parsedArray = JSON.parse<FixedArray<Nullish<string>>>(`["one","two"]`, newUndefiningReviver("0"), STRING_ARRAY_TYPE)
    arktest.assertEQ([null, "two"] as FixedArray<Nullish<string>>, parsedArray)

    return RESULT_SUCCESS
}

function jsonParseUsingReplacingReviver(): int {
    const NEW_X = 10.0

    const replacements = new Map<string, NullishType>()
    replacements.set("x", NEW_X)

    const point = new Point(3, 4)
    const parsedPoint: Point = JSON.parse<Point>(JSON.stringify(point), newReplacingReviver(replacements), POINT_TYPE)!

    const replacedPoint = new Point(NEW_X, point.y)
    arktest.assertEQ(replacedPoint.x, parsedPoint.x)
    arktest.assertEQ(replacedPoint.y, parsedPoint.y)

    const trash = new Trash(new Point(10, 20))
    const trashJson = JSON.stringify(trash)
    const trashType = typeFor([] as FixedArray<Trash>)

    let parsedTrash = JSON.parse<Trash>(trashJson, newUndefiningReviver("point"), trashType)

    arktest.assertTrue(parsedTrash != null)
    arktest.assertEQ(undefined, parsedTrash!.point)

    parsedTrash = JSON.parse<Trash>(trashJson, newNullingReviver("point"), trashType)

    arktest.assertTrue(parsedTrash != null)
    arktest.assertEQ(undefined, parsedTrash!.point)

    return RESULT_SUCCESS
}

function jsonParseArrayWithReplacingReviver(): int {
    const romans: FixedArray<Nullish<string>> = ["I", "II"]

    const replacements = new Map<string, NullishType>()

    const replacedRomans: FixedArray<Nullish<string>> = ["I"]
    replacements.set("", replacedRomans)

    const parsedRomans = JSON.parse<FixedArray<Nullish<string>>>(JSON.stringify(romans), newReplacingReviver(replacements), STRING_ARRAY_TYPE)
    arktest.assertEQ(parsedRomans, replacedRomans)

    return RESULT_SUCCESS
}

enum Color {
    Black = 0,
    Red = 1, 
    Green = 2
}

function consolePrint(color: Color, text: string): void {
    console.print("\x1b[38;5;" + color.toString() + "m")
    console.print(text)
    console.print("\x1b[0m")
}

class BooleanTestClass {
    flag: boolean = false;
}

function jsonParseWithBoolean(): int {
    let str = '{"flag":true}'
    let type = Type.from<BooleanTestClass>()
    let ret = JSON.parse<BooleanTestClass>(str, type) as BooleanTestClass
    arktest.assertEQ(ret.flag, true)
    return RESULT_SUCCESS
}

function main(): int {
    let failures = RESULT_SUCCESS

    failures += test(jsonParseUsingUndefinedReviver, "JSON.parse() using undefined 'reviver'")
    failures += test(jsonParseUsingRevivierWhichReturnsUndefined, "JSON.parse() using 'reviver' which returns 'undefined'")
    // #21831: ArrayStoreError: nullvalue cannot be stored in an array of type [Lstd/core/String;
    //failures += test(jsonParseArrayUsingReviverWhichReturnsUndefined, "JSON.parse(array) using 'reviver' which returns 'undefined'")
    // #27136: Nullish field type is UnionType which is not supported yet
    //failures += test(jsonParseUsingReplacingReviver, "JSON.parse() using replacing 'reviver'")
    failures += test(jsonParseArrayWithReplacingReviver, "JSON.parse(array) using replacing 'reviver'")
    failures += test(jsonParseWithBoolean, 'JSON.parse boolean in class')

    if (failures == RESULT_SUCCESS) {
        consolePrint(Color.Green, "PASSED: ")
        console.println("All tests run")
    } else {
        consolePrint(Color.Red, "FAILED: ")
        console.println("All tests run")
    }

    return failures
}

function test(testAction: () => int, testName: string): int {
    try {
        const testResult = testAction()

        if (testResult == RESULT_SUCCESS) {
            consolePrint(Color.Green, "PASSED: ")
            console.println(`"${testName}"`)
        } else {
            consolePrint(Color.Red, "FAILED: ")
            console.println(`"${testName}"`)
        }

        return testResult
    } catch (e) {
        consolePrint(Color.Red, "FAILED: ")
        if (e instanceof AssertionError) {
            const assertionError = e as AssertionError

            const errorMessage = assertionError.message
            console.println(`"${testName}" failed with error: ${errorMessage}`)
        } else {
            console.println(`"${testName}" failed with error ${e}`)
        }

        return RESULT_FAILURE
    }
}

function typeFor<T>(a: FixedArray<T>): Type {
    return (Type.of(a) as ArrayType).getElementType()
}
