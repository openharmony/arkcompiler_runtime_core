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
    const suite = new arktest.ArkTestsuite("JSON.stringify");

    suite.addTest("Stringify Date with negative number in constructor", stringifyNegativeDate);
    suite.addTest("Stringify object literal", stringifyObjectLiteral);

    return suite.run();
}

function stringifyNegativeDate() {
    let d = new Date(-271822)
    arktest.assertEQ(JSON.stringify(d), "\"1969-12-31T23:55:28.178Z\"")
}

class TestClass {
    value: string;
    constructor(v: string) { this.value = v; }
}

interface TestInterface {
    s: string;
    d: double;
    a: Any;
}

class TestInterfaceImpl implements TestInterface {
    s: string = "";
    d: double = 0;
    a: Any = undefined;
}

function stringifyObjectLiteral(): void {
    let obj: TestInterface = {s: "str", d: -1.5, a: undefined};
    arktest.assertTrue(Reflect.isLiteralInitializedInterface(obj));
    arktest.assertEQ(JSON.stringify(obj), '{"d":-1.5,"s":"str"}');

    obj.a = new TestClass("val")
    arktest.assertEQ(JSON.stringify(obj), '{"a":{"value":"val"},"d":-1.5,"s":"str"}');

    // Check that replacer is applied
    arktest.assertEQ(JSON.stringify(obj, (key: String, value: Any): Any => { return (key == "a") ? 123 : value }),
        '{"a":123,"d":-1.5,"s":"str"}')

    // Check that fields filter is not applied
    arktest.assertEQ(JSON.stringify(obj, ["s"]), '{"a":{},"d":-1.5,"s":"str"}')

    // Check cyclic dependencies are detected
    obj.a = obj;
    arktest.expectThrow(() => { JSON.stringify(obj) }, (e: Error) => e instanceof TypeError);

    // Check that getters are invoked only on object literals
    obj = new TestInterfaceImpl();
    arktest.assertEQ(JSON.stringify(obj), "{}");
}
