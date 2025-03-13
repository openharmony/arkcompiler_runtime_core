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

function dummyReplacer(key: string, value: NullishType): NullishType {
    return value
}

function toStringReplacer(key: string, value: NullishType): NullishType {
    if (key == "") {
        return value
    }

    return "" + value
}

class A {
    id: int = 1
    name = "A"

    constructor(id: int = 1) {
        this.id = id
    }
    
    toString(): string {
        return `${this.name}(${this.id})`
    }
}

class R {
    a = new A()
}

class E {}

class N {
    n = null
    u = undefined
}

function jsonStringifyObjectWithFunctionReplacer(): int {
    Assertions.assertEquals(`{"id":1,"name":"A"}`, JSON.stringify(new A(), dummyReplacer))
    Assertions.assertEquals(`{\n\t"id": 1,\n\t"name": "A"\n}`, JSON.stringify(new A(), dummyReplacer, "\t"))
    Assertions.assertEquals(`{\n "id": 1,\n "name": "A"\n}`, JSON.stringify(new A(), dummyReplacer, 1))
    Assertions.assertEquals(`{"id":"1","name":"A"}`, JSON.stringify(new A(), toStringReplacer))
    Assertions.assertEquals(`{"n":"null","u":"undefined"}`, JSON.stringify(new N(), toStringReplacer))

    return RESULT_SUCCESS
}

function jsonStringifyArrayWithFunctionReplacer(): int {
    const replacer = (key: string, value: NullishType): NullishType => {
        if (key == "" || key == "0") {
            return value
        } else {
            return undefined
        }
    }

    const array: Array<A> = Array.of(new A(), new A())
    Assertions.assertEquals(`[{"id":1,"name":"A"},{"id":1,"name":"A"}]`, JSON.stringify(array, dummyReplacer))
    Assertions.assertEquals(`["A(1)","A(1)"]`, JSON.stringify(array, toStringReplacer))
    Assertions.assertEquals(`[\n\t"A(1)",\n\t"A(1)"\n]`, JSON.stringify(array, toStringReplacer, "\t"))
    Assertions.assertEquals(`[\n "A(1)",\n "A(1)"\n]`, JSON.stringify(array, toStringReplacer, 1))
    Assertions.assertEquals(`[{},null]`, JSON.stringify(array, replacer))

    const ar = [new A(), new A()]
    Assertions.assertEquals(`[{"id":1,"name":"A"},{"id":1,"name":"A"}]`, JSON.stringify(ar, dummyReplacer))
    Assertions.assertEquals(`["A(1)","A(1)"]`, JSON.stringify(ar, toStringReplacer))
    Assertions.assertEquals(`[\n\t"A(1)",\n\t"A(1)"\n]`, JSON.stringify(ar, toStringReplacer, "\t"))
    Assertions.assertEquals(`[\n "A(1)",\n "A(1)"\n]`, JSON.stringify(ar, toStringReplacer, 1))
    Assertions.assertEquals(`[{},null]`, JSON.stringify(ar, replacer))

    const reverser = (key: string, value: NullishType): NullishType => 
            Array.isArray(value) ? Array.from(value as Object[]).reverse() : value

    Assertions.assertEquals(`[{"id":2,"name":"A"},{"id":1,"name":"A"}]`, JSON.stringify([new A(1), new A(2)], reverser))

    const numbers: number[] = [1, 2, 3]
    Assertions.assertEquals(`[1,2,3]`, JSON.stringify(numbers, dummyReplacer))
    Assertions.assertEquals(`["1","2","3"]`, JSON.stringify(numbers, toStringReplacer))

    Assertions.assertEquals("1", JSON.stringify(1, toStringReplacer))

    return RESULT_SUCCESS
}

const EMPTY_ARRAY: string[] = []

function jsonStringifyWithArrayReplacer(): int {
    Assertions.assertEquals(`{}`, JSON.stringify(new A(), EMPTY_ARRAY))
    Assertions.assertEquals(`{"id":1}`, JSON.stringify(new A(), ["id"]))
    Assertions.assertEquals(`{"name":"A","id":1}`, JSON.stringify(new A(), ["name", "id"]))

    Assertions.assertEquals(`[true]`, JSON.stringify([new Boolean(true)], EMPTY_ARRAY))

    const filter: (string | number)[] = ["id", 0]
    Assertions.assertEquals(`{"id":1}`, JSON.stringify(new A(), filter))

    // space cases
    Assertions.assertEquals(`{}`, JSON.stringify(new A(), EMPTY_ARRAY, "\t"))
    Assertions.assertEquals(`{\n\t"id": 1\n}`, JSON.stringify(new A(), ["id"], "\t"))
    Assertions.assertEquals(`{\n\t"a": {}\n}`, JSON.stringify(new R(), ["a"], "\t"))
    Assertions.assertEquals(`[\n\t{},\n\t{}\n]`, JSON.stringify([new A(), new A()], EMPTY_ARRAY, "\t"))

    return RESULT_SUCCESS
}

function jsonStringifyWithNullReplacer(): int {
    Assertions.assertEquals(`{"id":1,"name":"A"}`, JSON.stringify(new A(), null))
    Assertions.assertEquals(`{}`, JSON.stringify(new E(), null, "  "))

    return RESULT_SUCCESS
}

function jsonStringifyWithUndefinedReplacer(): int {
    Assertions.assertEquals(`{"id":1,"name":"A"}`, JSON.stringify(new A(), undefined))

    return RESULT_SUCCESS
}

function main(): int {
    let failures = 0

    failures += test(jsonStringifyObjectWithFunctionReplacer, "JSON.stringify object with function replacer")
    failures += test(jsonStringifyArrayWithFunctionReplacer, "JSON.stringify array with function replacer")
    failures += test(jsonStringifyWithArrayReplacer, "JSON.stringify with array replacer")
    failures += test(jsonStringifyWithNullReplacer, "JSON.stringify with 'null' replacer")
    failures += test(jsonStringifyWithUndefinedReplacer, "JSON.stringify with 'undefined' replacer")

    if (failures == RESULT_SUCCESS) {
        console.println("PASSED: All tests run")
    } else {
        console.println("FAILED: All tests run")
    }

    return failures
}

function test(testAction: () => int, testName: string): int {
    try {
        const testResult = testAction()

        if (testResult == RESULT_SUCCESS) {
            console.println(`PASSED: "${testName}"`)
        } else {
            console.println(`FAILED: "${testName}"`)
        }

        return testResult
    } catch (e) {
        if (e instanceof AssertionError) {
            const assertionError = e as AssertionError

            const errorMessage = assertionError.message
            console.println(`FAILED: "${testName}" failed with error: ${errorMessage}`)
        } else {
            console.println(`FAILED: "${testName}" failed with exception ${e}`)
        }

        return RESULT_FAILURE
    }
}

class Assertions {
    static assertEquals(expected: string, actual: string): void {
        assertEQ(expected,  actual,  `expected "${expected}" but was "${actual}"`)
    }
}