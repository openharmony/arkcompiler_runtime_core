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

class C {
    age: number = 0
    name: string = ""
}

class O<T> implements JsonReplacer {
    private buffer: Array<T>
    private capacity: number = 20

    constructor(capacity: number) {
        this.capacity = capacity;
        this.buffer = new Array<T>(this.capacity)
        this.buffer.length = 0
    }

    pushBack(val: T): void {
        this.buffer.push(val)
    }

    jsonReplacer(): Record<String, NullishType> {
        const buf = this.buffer
        const len = buf.length
        let arrayObj: Record<String, NullishType> = {}
        for (let i = 0; i < len; i++) {
            arrayObj[String(i)] = buf[i]
        }
        return arrayObj
    }
}

class D<T> {
    private value: O<T>
    private capacity: number

    constructor(capacity: number) {
        this.capacity = capacity
        this.value = new O<T>(capacity)
    }

    pushBack(val: T): void {
        this.value.pushBack(val)
    }
}

function jsonStringifyObjectWithFunctionReplacer(): void {
    assertEQ(JSON.stringify(new A(), dummyReplacer), `{"id":1,"name":"A"}`)
    assertEQ(JSON.stringify(new A(), dummyReplacer, "\t"), `{\n\t"id": 1,\n\t"name": "A"\n}`)
    assertEQ(JSON.stringify(new A(), dummyReplacer, 1), `{\n "id": 1,\n "name": "A"\n}`)
    assertEQ(JSON.stringify(new A(), toStringReplacer), `{"id":"1","name":"A"}`)
    assertEQ(JSON.stringify(new N(), toStringReplacer), `{"n":"null","u":"undefined"}`)
}

function jsonStringifyRecordWithFunctionReplacer(): void {
    const record: Record<string, NullishType> = { "id": 1, "name": "R" }
    assertEQ(JSON.stringify(record, dummyReplacer), `{"id":1,"name":"R"}`)
}

function jsonStringifyArrayWithFunctionReplacer(): void {
    const replacer = (key: string, value: NullishType): NullishType => {
        if (key == "" || key == "0") {
            return value
        } else {
            return undefined
        }
    }

    const array: Array<A> = Array.of(new A(), new A())
    assertEQ(JSON.stringify(array, dummyReplacer), `[{"id":1,"name":"A"},{"id":1,"name":"A"}]`)
    assertEQ(JSON.stringify(array, toStringReplacer), `["A(1)","A(1)"]`)
    assertEQ(JSON.stringify(array, toStringReplacer, "\t"), `[\n\t"A(1)",\n\t"A(1)"\n]`)
    assertEQ(JSON.stringify(array, toStringReplacer, 1), `[\n "A(1)",\n "A(1)"\n]`)
    assertEQ(JSON.stringify(array, replacer), `[{},null]`)

    const ar = [new A(), new A()]
    assertEQ(JSON.stringify(ar, dummyReplacer), `[{"id":1,"name":"A"},{"id":1,"name":"A"}]`)
    assertEQ(JSON.stringify(ar, toStringReplacer), `["A(1)","A(1)"]`)
    assertEQ(JSON.stringify(ar, toStringReplacer, "\t"), `[\n\t"A(1)",\n\t"A(1)"\n]`)
    assertEQ(JSON.stringify(ar, toStringReplacer, 1), `[\n "A(1)",\n "A(1)"\n]`)
    assertEQ(JSON.stringify(ar, replacer), `[{},null]`)

    const reverser = (key: string, value: NullishType): NullishType => 
            Array.isArray(value) ? Array.from(value as FixedArray<Object>).reverse() : value

    assertEQ(JSON.stringify([new A(1), new A(2)] as FixedArray<A>, reverser), `[{"id":2,"name":"A"},{"id":1,"name":"A"}]`)

    const numbers: FixedArray<number> = [1, 2, 3]
    assertEQ(JSON.stringify(numbers, dummyReplacer), `[1,2,3]`)
    assertEQ(JSON.stringify(numbers, toStringReplacer), `["1","2","3"]`)

    assertEQ(JSON.stringify(1, toStringReplacer), "1")
}

const EMPTY_ARRAY: FixedArray<string> = []

function jsonStringifyWithArrayReplacer(): void {
    const rec: Record<string, NullishType> = { "id": 1, "name": "R" }
    assertEQ(JSON.stringify(new A(), EMPTY_ARRAY), `{}`)
    assertEQ(JSON.stringify(rec, EMPTY_ARRAY), `{}`)
    assertEQ(JSON.stringify(new A(), ["id"]), `{"id":1}`)
    assertEQ(JSON.stringify(rec, ["id"]), `{"id":1}`)
    assertEQ(JSON.stringify(new A(), ["name", "id"]), `{"name":"A","id":1}`)
    assertEQ(JSON.stringify(rec, ["name", "id"]), `{"name":"R","id":1}`)

    assertEQ(JSON.stringify([new Boolean(true)], EMPTY_ARRAY), `[true]`)

    const filter: FixedArray<string | number> = ["id", 0]
    assertEQ(JSON.stringify(new A(), filter), `{"id":1}`)
    assertEQ(JSON.stringify(rec, filter), `{"id":1}`)

    // space cases
    assertEQ(JSON.stringify(new A(), EMPTY_ARRAY, "\t"), `{}`)
    assertEQ(JSON.stringify(new A(), ["id"], "\t"), `{\n\t"id": 1\n}`)
    assertEQ(JSON.stringify(rec, ["id"], "\t"), `{\n\t"id": 1\n}`)
    assertEQ(JSON.stringify(new R(), ["a"], "\t"), `{\n\t"a": {}\n}`)
    assertEQ(JSON.stringify([new A(), new A()], EMPTY_ARRAY, "\t"), `[\n\t{},\n\t{}\n]`)
    assertEQ(JSON.stringify([rec, rec], EMPTY_ARRAY, "\t"), `[\n\t{},\n\t{}\n]`)
}

function jsonStringifyWithNullReplacer(): void {
    assertEQ(JSON.stringify(new A(), null), `{"id":1,"name":"A"}`)
    const rec: Record<string, NullishType> = { "id": 1, "name": "R" }
    assertEQ(JSON.stringify(rec, null), `{"id":1,"name":"R"}`)
    assertEQ(JSON.stringify(new E(), null, "  "), `{}`)
    const emptyRec: Record<string, NullishType> = {}
    assertEQ(JSON.stringify(emptyRec, null, "  "), `{}`)
}

function jsonStringifyWithUndefinedReplacer(): void {
    assertEQ(JSON.stringify(new A(), undefined), `{"id":1,"name":"A"}`)
    const rec: Record<string, NullishType> = { "id": 1, "name": "R" }
    assertEQ(JSON.stringify(rec, undefined), `{"id":1,"name":"R"}`)
}

function jsonStringifyWithJsonReplacer(): void {
    let testClass: O<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>
        = new O<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>(20)
    testClass.pushBack("zhang san")
    testClass.pushBack(1)
    testClass.pushBack({
        age: 10, name: "zhang"
    } as C)
    testClass.pushBack(undefined)
    testClass.pushBack(null)
    testClass.pushBack(true)
    let jsonResult = JSON.stringify(testClass)
    let result = `{"0":"zhang san","1":1,"2":{"age":10,"name":"zhang"},"4":null,"5":true}`
    assertEQ(jsonResult, result, `{"0":"zhang san","1":1,"2":{"age":10,"name":"zhang"},"4":null,"5":true}`)
}

function jsonStringifyWithNestedReplacer(): void {
    let testClass: D<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>
        = new D<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>(20)
    testClass.pushBack("zhang san")
    testClass.pushBack(1)
    testClass.pushBack({
        age: 10, name: "zhang"
    } as C)
    testClass.pushBack(undefined)
    testClass.pushBack(null)
    testClass.pushBack(true)
    let jsonResult = JSON.stringify(testClass)
    let result = `{"value":{"0":"zhang san","1":1,"2":{"age":10,"name":"zhang"},"4":null,"5":true},"capacity":20}`
    assertEQ(jsonResult, result, `{"value":{"0":"zhang san","1":1,"2":{"age":10,"name":"zhang"},"4":null,"5":true},"capacity":20}`)
}

function jsonStringifyWithSpaceReplacer(): void {
    let testClass: O<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>
        = new O<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>(20)
    testClass.pushBack("zhang san")
    testClass.pushBack(1)
    testClass.pushBack({
        age: 10, name: "zhang"
    } as C)
    testClass.pushBack(undefined)
    testClass.pushBack(null)
    testClass.pushBack(true)
    let jsonResult = JSON.stringify(testClass, undefined, " ")
    let result = `{\n "0": "zhang san",\n "1": 1,\n "2": {\n  "age": 10,\n  "name": "zhang"\n },\n "4": null,\n "5": true\n}`
    assertEQ(jsonResult, result, `{\n "0": "zhang san",\n "1": 1,\n "2": {\n  "age": 10,\n  "name": "zhang"\n },\n "4": null,\n "5": true\n}`)
}

function jsonStringifyWithReplacer(): void {
    let testClass: O<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>
        = new O<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>(20)
    testClass.pushBack("zhang san")
    testClass.pushBack(1)
    testClass.pushBack({
        age: 10, name: "zhang"
    } as C)
    testClass.pushBack(undefined)
    testClass.pushBack(null)
    testClass.pushBack(true)
    let jsonResult = JSON.stringify(testClass, ["0"], " ")
    let result = `{\n "0": "zhang san"\n}`
    assertEQ(jsonResult, result, `{\n "0": "zhang san"\n}`)
}

function main(): int {
    const suite = new ArkTestsuite("JSON.stringify")
    suite.addTest("object with function replacer", jsonStringifyObjectWithFunctionReplacer)
    suite.addTest("record with function replacer", jsonStringifyRecordWithFunctionReplacer)
    suite.addTest("array with function replacer", jsonStringifyArrayWithFunctionReplacer)
    suite.addTest("class implements JsonReplacer", jsonStringifyWithJsonReplacer)
    suite.addTest("class implements JsonReplacer nested case", jsonStringifyWithNestedReplacer)
    suite.addTest("class implements JsonReplacer with space", jsonStringifyWithSpaceReplacer)
    suite.addTest("class implements JsonReplacer with space and replacer", jsonStringifyWithReplacer)
    suite.addTest("with array replacer", jsonStringifyWithArrayReplacer)
    suite.addTest("with 'null' replacer", jsonStringifyWithNullReplacer)
    suite.addTest("with 'undefined' replacer", jsonStringifyWithUndefinedReplacer)
    return suite.run()
}
