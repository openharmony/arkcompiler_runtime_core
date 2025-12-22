/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
    const suite = new arktest.ArkTestsuite("JSON.stringify")

    suite.addTest("Stringify Date with negative number in constructor", stringifyNegativeDate)
    suite.addTest("Stringify object literal - simple cases", stringifyObjectLiteralSimpleCases)
    suite.addTest("Stringify object literal - corner cases", stringifyObjectLiteralCornerCases)

    return suite.run()
}

function stringifyNegativeDate() {
    let d = new Date(-271822)
    arktest.assertEQ(JSON.stringify(d), "\"1969-12-31T23:55:28.178Z\"")
}

class TestClass {
    value: string;
    constructor(v: string) { this.value = v }
}

interface TestInterface {
    s: string
    d: double
    a: Any
}

class TestInterfaceImpl implements TestInterface {
    s: string = ""
    d: double = 0
    a: Any = undefined
}

function checkJSONObject(jObj: JSONObject, expectedValues: Map<string, string>) {
    const flds = jObj.getFields()
    arktest.assertEQ(flds.size, expectedValues.size)
    for (const entry of expectedValues.entries()) {
        arktest.assertEQ(flds.get(entry[0])?.toString(), entry[1])
    }
}

function checkJSONArray(jsonArr: JSONArray, expectedValues: Map<string, string>[]) {
    const jsonArrValues = jsonArr.values
    arktest.assertEQ(jsonArrValues.length, expectedValues.length)
    for (let i = 0; i < jsonArrValues.length; i++) {
        checkJSONObject(jsonArrValues[i] as JSONObject, expectedValues[i])
    }
}

function stringifyObjectLiteralSimpleCases(): void {
    let obj: TestInterface = {s: "str", d: -1.5, a: undefined}
    arktest.assertTrue(Reflect.isLiteralInitializedInterface(obj))

    // Expect {"d":-1.5,"s":"str"}
    let jsonStr = JSON.stringify(obj)
    let expected: Map<string, string> = new Map<string,string>([
        ["d", "-1.5"],
        ["s", "str"],
    ])
    checkJSONObject(JSONParser.parse(jsonStr) as JSONObject, expected)

    obj.a = new TestClass("val")
    // Expect {"a":{"value":"val"},"d":-1.5,"s":"str"}
    jsonStr = JSON.stringify(obj)
    expected.set("a", "{value:val}")
    checkJSONObject(JSONParser.parse(jsonStr) as JSONObject, expected)

    let array: TestInterface[] = [{s: "", d: 0.0, a: undefined}, {s: "s", d: 1.5, a: new TestClass("val")}]
    // Expect [{"d":0,"s":""},{"a":{"value":"val"},"d":1.5,"s":"s"}]
    jsonStr = JSON.stringify(array)
    expected.set("d", "1.5")
    expected.set("s", "s")
    const expectedArray: Map<string,string>[] = [new Map<string,string>([["d", "0"], ["s", ""]]), expected]
    checkJSONArray(JSONParser.parse(jsonStr) as JSONArray, expectedArray)

    let fixedArray: FixedArray<TestInterface> = [{s: "", d: 0.0, a: undefined}, {s: "s", d: 1.5, a: new TestClass("val")}]
    // Expect [{"d":0,"s":""},{"a":{"value":"val"},"d":1.5,"s":"s"}]
    jsonStr = JSON.stringify(fixedArray)
    checkJSONArray(JSONParser.parse(jsonStr) as JSONArray, expectedArray)

    let record: Record<string, TestInterface[]> = new Record<string, TestInterface[]>()
    record["filler"] = array
    // Expect {"filler":[{"d":0,"s":""},{"a":{"value":"val"},"d":-1.5,"s":"str"}]}
    jsonStr = JSON.stringify(record)
    const jsonObj = JSONParser.parse(jsonStr) as JSONObject
    const fields = jsonObj.getFields()
    arktest.assertEQ(fields.size, 1)
    checkJSONArray(fields.get("filler") as JSONArray, expectedArray)
}

function stringifyObjectLiteralCornerCases(): void {
    const testObj = new TestClass("val")
    let objLit: TestInterface = {s: "str", d: -1.5, a: testObj}
    arktest.assertTrue(Reflect.isLiteralInitializedInterface(objLit))

    // Expect {"a":{"value":"val"},"d":-1.5,"s":"str"}
    let jsonStr = JSON.stringify(objLit)
    let expected: Map<string, string> = new Map<string,string>([
        ["a", "{value:val}"],
        ["d", "-1.5"],
        ["s", "str"],
    ])
    checkJSONObject(JSONParser.parse(jsonStr) as JSONObject, expected)

    // Check that replacer is applied
    // Expect {"a":123,"d":-1.5,"s":"str"}
    jsonStr = JSON.stringify(objLit, (key: String, value: Any): Any => { return (key == "a") ? 123 : value })
    expected.set("a", "123")
    checkJSONObject(JSONParser.parse(jsonStr) as JSONObject, expected)

    // Check that fields filter is not applied to object literal itself, but is applied to all nested objects
    // Expect {"a":{},"d":-1.5,"s":"str"}
    jsonStr = JSON.stringify(objLit, ["s"])
    expected.set("a", "{}")
    checkJSONObject(JSONParser.parse(jsonStr) as JSONObject, expected)

    // Check cyclic dependencies are detected
    objLit.a = objLit;
    arktest.expectThrow(() => { JSON.stringify(objLit) }, (e: Error) => e instanceof TypeError)
    let array: TestInterface[] = [objLit]
    arktest.expectThrow(() => { JSON.stringify(array) }, (e: Error) => e instanceof TypeError)
    let fixedArray: FixedArray<TestInterface> = [objLit]
    arktest.expectThrow(() => { JSON.stringify(fixedArray) }, (e: Error) => e instanceof TypeError)
    objLit.a = testObj

    // Check that getters are invoked only on object literals
    let obj = new TestInterfaceImpl()
    arktest.assertEQ(JSON.stringify(obj), "{}")

    array = [obj]
    arktest.assertEQ(JSON.stringify(array), "[{}]")

    array = [objLit, obj]
    jsonStr = JSON.stringify(array)
    expected.set("a", "{value:val}")
    checkJSONArray(JSONParser.parse(jsonStr) as JSONArray, [expected, new Map<string, string>()])

    fixedArray = [obj]
    arktest.assertEQ(JSON.stringify(fixedArray), "[{}]")

    fixedArray = [objLit, obj]
    jsonStr = JSON.stringify(fixedArray)
    checkJSONArray(JSONParser.parse(jsonStr) as JSONArray, [expected, new Map<string, string>()])
}
