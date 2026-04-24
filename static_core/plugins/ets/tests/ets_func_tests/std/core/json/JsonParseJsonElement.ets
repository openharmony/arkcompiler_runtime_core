/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
    const suite = new arktest.ArkTestsuite("JSON.parseJsonElement tests")
    suite.addTest("call parseJsonElement using simple string argument", stringTests)
    suite.addTest("call parseJsonElement using incorrect string argument", negativeTests)
    suite.addTest("send parseJsonElement result into stringifyJsonElement and compare", parseToStringifyTests)
    suite.addTest("call parseJsonElement with reviver", reviverTests)

    suite.addTest("handle scientific notation values", scientificNotationTest)
    suite.addTest("handle unicode keys", unicodeKeysTest)
    suite.addTest("count depth in deeply nested structures", deeplyNestedTest)
    suite.addTest("mixed array with various types", mixedArrayTest)
    suite.addTest("special ASCII characters in keys", specialCharactersTest)

    suite.addTest("empty objects, arrays and null", emptyValuesTest)
    suite.addTest("string escape sequences", escapeSequencesTest)
    suite.addTest("Number special values", numberSpecialValuesTest)
    suite.addTest("delete property by returning JsonNull", deleteKeywordTest)
    suite.addTest("array boundary values", arrayBoundaryTest)
    suite.addTest("verify if original or decoded keys are passed", unicodeKeysTest2)

    return suite.run()
}

function stringTests(): void {
    const testData: FixedArray<[string, (elem: jsonx.JsonElement) => boolean]> = [
        [`[]`, elem => elem.asArray().length == 0],
        [`[null, "a", "b"]`, elem => elem[0].jsonType == jsonx.JsonType.JsonNull],
        [`[-1, -3, -5]`, elem => elem[2].asInteger() == -5],
        [`[{"id": 1, "name": "A"}, {"id": 2, "name": "B"}]`, elem => elem[1]["name"].asString() == "B"],
        [`[-1.5, -2.5, -3.5]`, elem => elem[2].asDouble() == -3.5],
        [`[0, 0, 0]`, elem => elem[2].asInteger() == 0],
        [`{"list": []}`, elem => elem["list"].asArray().length == 0],
        [`[[[1], [2]], [[3], [4]]]`, elem => elem[1][0][0].asInteger() == 3],
        [`"\\u263A\\u270C"`, elem => elem.asString() == "☺✌"],
        [`[-0, 0, -0]`, elem => elem[0].asDouble() == -0],
        [`["function a(){}", "function b(){}"]`, elem => elem[1].asString() == "function b(){}"],
        [`[1, [], [2, 3]]`, elem => elem[1].asArray().length == 0],
        [`[1, null, 3, null, 5]`, elem => elem[3].jsonType == jsonx.JsonType.JsonNull],
        [`[[[[[10], 20], 30], 40], 50]`, elem => elem[0][0][0][0][0].asInteger() == 10],
        [`"\\u2603\\u2600"`, elem => elem.asString() == "☃☀"],
        [`["NaN", "123", "456"]`, elem => elem[0].asString() == "NaN"],
        [`["-Infinity", "0", "Infinity"]`, elem => elem[0].asString() == "-Infinity"],
        [`[[1, -0], [2, 3]]`, elem => elem[0][1].asDouble() == -0],
        [`[[], [1, []], [2, [3, []]]]`, elem => elem[1][1].asArray().length == 0],
        [`"\\u5E26\\u7B26\\u53F7\\u7684\\u5B57\\u7B26\\u4E32：!@#"`, elem => elem.asString() == "带符号的字符串：!@#"],
        [`[1,2,3]`, elem => elem[1].asInteger() == 2],
        [`[{"k":1}]`, elem => elem[0]["k"].asDouble() == 1],
        [`{"a":1,"b":"test","c":[1,2,3]}`, elem => Array.from(elem.$_iterator()).length == 3],
        [`true`, elem => elem.jsonType == jsonx.JsonType.JsonTrue],
        [`false`, elem => elem.jsonType == jsonx.JsonType.JsonFalse],
        [`null`, elem => elem.jsonType == jsonx.JsonType.JsonNull],
        [`[1,2,3,]`, elem => {
            const arr: jsonx.JsonElement[] = elem.asArray()
            return arr.length == 3 &&
                arr[0].tryAsInteger() == 1 &&
                arr[1].tryAsInteger() == 2 &&
                arr[2].tryAsInteger() == 3
        }],
        [`[0,"",false,null,[],{}]`, elem => {
            const arr: jsonx.JsonElement[] = elem.asArray()
            return arr.length == 6 &&
                arr[0].tryAsInteger() == 0 &&
                arr[1].asString() == "" &&
                arr[2].jsonType == jsonx.JsonType.JsonFalse &&
                arr[3].jsonType == jsonx.JsonType.JsonNull &&
                arr[4].jsonType == jsonx.JsonType.JsonArray &&
                arr[5].jsonType == jsonx.JsonType.JsonObject
        }],
        [`[1e3,2.5e-2,1.5E+10,-3.5e2]`, elem => {
            const arr: jsonx.JsonElement[] = elem.asArray()
            return arr[0].tryAsDouble() == 1000 &&
                arr[1].tryAsDouble() == 0.025 &&
                arr[2].tryAsDouble() == 15000000000 &&
                arr[3].tryAsDouble() == -350
        }],
        [`{"a":1,"a":2,"a":3}`, elem => elem["a"].asDouble() == 3]
    ]

    for (const data of testData) {
        const str = data[0]
        const check = data[1]
        const result: jsonx.JsonElement = JSON.parseJsonElement(str)
        const success = check(result)
        arktest.assertTrue(success, `JSON.parseJsonElement("${str}") failed`)
    }
}

function negativeTests(): void {
    const testData: FixedArray<string> = [
        `{"a":1,"b":2`,
        `{"invalid":"\\uGGGG"}`,
        `{'a':1}`,
        `{"a":undefined}`,
        `   \n\t  `,
        `[1,,2]`,
        `[{\"a\":1}`,
        `9223372036854775808`,
    ]

    for (const str of testData) {
        try {
            JSON.parseJsonElement(str)
        } catch (e) {
            continue
        }
        arktest.assertTrue(false, `JSON.parseJsonElement("${str}") expected to fail`)
    }
}

function parseToStringifyTests(): void {
    const testData: FixedArray<string> = [
        `[42,"text",true,false,null,{"nested":"obj"},[1,2,3]]`,
        `[72,101,108,108,111,0,0,0,-111,114,108,100]`,
        `{"a":1,"b":2}`,
        `{"data":"${"x".repeat(10000)}"}`,
        `{"special":"!#$%&'()*+,-./:;<=>?@[]^_\`{|}~"}`,
        createDeepNested(),
    ]

    for (const str of testData) {
        const elem: jsonx.JsonElement = JSON.parseJsonElement(str)
        const result: string = JSON.stringifyJsonElement(elem)
        arktest.assertEQ(str, result)
    }
}

function createDeepNested(): string {
    let str: string = "1"
    for (let i = 0; i < 50; i++) {
        str = `{"a":${str}}`
    }
    return str
}

function reviverTests(): void {
    const testData: FixedArray<[string, string, (key: string, value: jsonx.JsonElement) => jsonx.JsonElement]> = [
        [`{"a":5,"b":10}`, `{"a":10,"b":20}`, mul2],
        [`{"name":"hello","type":"world"}`, `{"name":"HELLO","type":"WORLD"}`, uppercase],
        [`{"level1":{"level2":{"value":42}}}`, `{"level1":{"level2":{"value":999}}}`, num999],
        [`{"a":1,"b":null,"c":3}`, `{"a":1,"b":"filtered","c":3}`, filterNull],
        [`[1,2,3,4,5]`, `[10,2,30,4,50]`, ifEvenMul10],
        [`{"flag1":true,"flag2":false,"flag3":true}`, `{"flag1":false,"flag2":true,"flag3":false}`, switchBoolean],
    ]

    for (const data of testData) {
        const str = data[0]
        const expected = data[1]
        const reviver = data[2]
        const elem: jsonx.JsonElement = JSON.parseJsonElement(str, reviver)
        const result: string = JSON.stringifyJsonElement(elem)
        arktest.assertEQ(result, expected)
    }
}

function mul2(key: string, value: jsonx.JsonElement): jsonx.JsonElement {
    if (key === "") {
        return value
    }
    if (value.jsonType === jsonx.JsonType.JsonNumber) {
        const num: int | undefined = value.tryAsInteger()
        if (num !== undefined) {
            return jsonx.JsonElement.createInteger(num * 2)
        }
    }
    return value
}

function uppercase(key: string, value: jsonx.JsonElement): jsonx.JsonElement {
    if (key === "") {
        return value
    }
    if (value.jsonType === jsonx.JsonType.JsonString) {
        const str: string = value.asString()
        return jsonx.JsonElement.createString(str.toUpperCase())
    }
    return value
}

function num999(key: string, value: jsonx.JsonElement): jsonx.JsonElement {
    if (key === "value" && value.jsonType === jsonx.JsonType.JsonNumber) {
        return jsonx.JsonElement.createInteger(999)
    }
    return value
}

function filterNull(key: string, value: jsonx.JsonElement): jsonx.JsonElement {
    if (key === "") {
        return value
    }
    if (value.jsonType === jsonx.JsonType.JsonNull) {
        return jsonx.JsonElement.createString("filtered")
    }
    return value
}

function ifEvenMul10(key: string, value: jsonx.JsonElement): jsonx.JsonElement {
    if (key === "") {
        return value
    }
    if (value.jsonType === jsonx.JsonType.JsonNumber) {
        const idx: int = parseInt(key).toInt()
        const num: int | undefined = value.tryAsInteger()
        if (num !== undefined && idx % 2 === 0) {
            return jsonx.JsonElement.createInteger(num * 10)
        }
    }
    return value
}

function switchBoolean(key: string, value: jsonx.JsonElement): jsonx.JsonElement {
    if (key === "") {
        return value
    }
    if (value.jsonType === jsonx.JsonType.JsonTrue) {
        return jsonx.JsonElement.createBoolean(false)
    }
    if (value.jsonType === jsonx.JsonType.JsonFalse) {
        return jsonx.JsonElement.createBoolean(true)
    }
    return value
}

function scientificNotationTest(): void {
    const str: string = "[1e-10,1e10]"
    let callLog: string[] = []

    const reviver = (key: string, value: jsonx.JsonElement): jsonx.JsonElement => {
        if (key !== "") {
            const typeName: string = jsonx.JsonType[value.jsonType]
            const num: double | undefined = value.tryAsDouble()
            callLog.push(`key=${key},type=${typeName},value=${num}`)

            if (key === "0" && value.jsonType === jsonx.JsonType.JsonNumber) {
                return jsonx.JsonElement.createInteger(999)
            }
            if (key === "1" && value.jsonType === jsonx.JsonType.JsonNumber) {
                return jsonx.JsonElement.createInteger(-888)
            }
        }
        return value
    }
    const elem: jsonx.JsonElement = JSON.parseJsonElement(str, reviver)

    arktest.assertEQ(callLog.length, 2)
    arktest.assertTrue(callLog[0].includes("key=0"))
    arktest.assertTrue(callLog[1].includes("key=1"))

    const result: string = JSON.stringifyJsonElement(elem)
    arktest.assertEQ(result, "[999,-888]")
}

function unicodeKeysTest(): void {
    const str: string = `{"\\u004b\\u0065\\u0079":123}`
    const keysSeen: string[] = []

    const reviver = (key: string, value: jsonx.JsonElement): jsonx.JsonElement => {
        keysSeen.push(key)
        return value
    }
    const elem: jsonx.JsonElement = JSON.parseJsonElement(str, reviver)

    arktest.assertTrue(keysSeen.length > 0)
    const result: string = JSON.stringifyJsonElement(elem)
    arktest.assertEQ(result, `{"Key":123}`)
}

function deeplyNestedTest(): void {
    let str: string = `{"value":1}`
    for (let i: number = 0; i < 10; i++) {
        str = `{"a":${str}}`
    }

    let maxDepth: int = 0
    const reviver = (key: string, value: jsonx.JsonElement): jsonx.JsonElement => {
        if (key === "a") {
            maxDepth++
        }
        return value
    }
    const elem: jsonx.JsonElement = JSON.parseJsonElement(str, reviver)

    arktest.assertTrue(maxDepth >= 10)
    const result: string = JSON.stringifyJsonElement(elem)
    arktest.assertTrue(result.length > 0)
}

function mixedArrayTest(): void {
    const str: string = `[42,"text",true,false,null,{"nested":1},[1,2]]`
    const typeCount: Map<string, int> = new Map<string, int>()

    const reviver = (key: string, value: jsonx.JsonElement): jsonx.JsonElement => {
        if (key !== "") {
            const typeName: string = jsonx.JsonType[value.jsonType]
            const current: int = typeCount.get(typeName) ?? 0
            typeCount.set(typeName, current + 1)
        }
        return value
    }
    const elem: jsonx.JsonElement = JSON.parseJsonElement(str, reviver)

    arktest.assertTrue(typeCount.size >= 5)
    const result: string = JSON.stringifyJsonElement(elem)
    arktest.assertTrue(result.startsWith("["))
}

function specialCharactersTest(): void {
    const str: string = `{"key_with_space":1,"key-with-dash":2,"key.with.dot":3}`
    const keys: string[] = []

    const reviver = (key: string, value: jsonx.JsonElement): jsonx.JsonElement => {
        if (key !== "") {
            keys.push(key)
        }
        return value
    }
    const elem: jsonx.JsonElement = JSON.parseJsonElement(str, reviver)

    arktest.assertEQ(keys.length, 3)
    arktest.assertEQ(keys[0], "key_with_space")
    arktest.assertEQ(keys[1], "key-with-dash")
    arktest.assertEQ(keys[2], "key.with.dot")
}

function emptyValuesTest(): void {
    const str: string = `{"emptyObj":{},"emptyArr":[],"nullVal":null}`
    let nullCount: int = 0
    let emptyObjCount: int = 0
    let emptyArrCount: int = 0

    const reviver = (key: string, value: jsonx.JsonElement): jsonx.JsonElement => {
        if (value.jsonType === jsonx.JsonType.JsonNull) {
            nullCount++
        } else if (value.jsonType === jsonx.JsonType.JsonObject) {
            const it = value.$_iterator()
            if (Array.from(it).length === 0) {
                emptyObjCount++
            }
        } else if (value.jsonType === jsonx.JsonType.JsonArray) {
            if (value.asArray().length === 0) {
                emptyArrCount++
            }
        }
        return value
    }
    const elem: jsonx.JsonElement = JSON.parseJsonElement(str, reviver)

    arktest.assertEQ(nullCount, 1)
    arktest.assertEQ(emptyObjCount, 1)
    arktest.assertEQ(emptyArrCount, 1)
}

function escapeSequencesTest(): void {
    const str: string = `{"newline":"line1\\nline2","tab":"col1\\tcol2","quote":"say \\"hello\\"","backslash":"path\\\\file"}`
    const escapeLog: string[] = []

    const reviver = (key: string, value: jsonx.JsonElement): jsonx.JsonElement => {
        if (key !== "" && value.jsonType === jsonx.JsonType.JsonString) {
            const str: string = value.asString()
            escapeLog.push(`${key}=${str.length}`)
        }
        return value
    }
    const elem: jsonx.JsonElement = JSON.parseJsonElement(str, reviver)

    arktest.assertEQ(escapeLog.length, 4)
    const result: string = JSON.stringifyJsonElement(elem)
    arktest.assertTrue(result.includes("line1\\nline2"))
    arktest.assertTrue(result.includes("col1\\tcol2"))
}

function numberSpecialValuesTest(): void {
    const str: string = "[42,null]"
    const typeLog: string[] = []

    const reviver = (key: string, value: jsonx.JsonElement): jsonx.JsonElement => {
        if (key !== "") {
            typeLog.push(`${key}:${jsonx.JsonType[value.jsonType]}`)
        }
        return value
    }
    const elem: jsonx.JsonElement = JSON.parseJsonElement(str, reviver)

    arktest.assertEQ(typeLog.length, 2)
    arktest.assertTrue(typeLog[0].includes("Number"))
    arktest.assertTrue(typeLog[1].includes("Null"))
}

function deleteKeywordTest(): void {
    const str: string = `{"a":1,"b":2,"c":3}`

    const reviver = (key: string, value: jsonx.JsonElement): jsonx.JsonElement => {
        if (key === "b") {
            return jsonx.JsonElement.createNull()
        }
        return value
    }
    const elem: jsonx.JsonElement = JSON.parseJsonElement(str, reviver)

    const result: string = JSON.stringifyJsonElement(elem)
    arktest.assertTrue(result.includes(`"b"`))
}

function arrayBoundaryTest(): void {
    const str: string = "[0, -1, 256, -32769, 1.7976931348623157e+308]"
    const boundaryTypes: string[] = []

    const reviver = (key: string, value: jsonx.JsonElement): jsonx.JsonElement => {
        if (key !== "" && value.jsonType === jsonx.JsonType.JsonNumber) {
            if (value.tryAsInteger() !== undefined) {
                boundaryTypes.push("integer")
            } else if (value.tryAsDouble() !== undefined) {
                boundaryTypes.push("double")
            }
        }
        return value
    }
    const elem: jsonx.JsonElement = JSON.parseJsonElement(str, reviver)

    arktest.assertEQ(boundaryTypes.length, 5)
    const result: string = JSON.stringifyJsonElement(elem)
    arktest.assertTrue(result.startsWith("["))
}

function unicodeKeysTest2(): void {
    const str: string = `{"\\u004b\\u0065\\u0079":"value"}`
    let receivedKey: string = ""

    const reviver = (key: string, value: jsonx.JsonElement): jsonx.JsonElement => {
        if (key !== "") {
            receivedKey = key
        }
        return value
    }
    const elem: jsonx.JsonElement = JSON.parseJsonElement(str, reviver)

    arktest.assertEQ(receivedKey, "Key")
    const result: string = JSON.stringifyJsonElement(elem)
    arktest.assertEQ(result, `{"Key":"value"}`)
}
