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

/*---
desc: >-
    If a class has accessor in a form of setter then its name can be used
    as a part of an object literal. The setter is invoked during initialization.
---*/

class WithGetterAndSetter {
    private _value: number = 0
    set value(v: number) { this._value = v * 2 }
    get value(): number { return this._value }
}

class SetterOnly {
    private _data: string = ""
    set data(s: string) { this._data = s }
    getData(): string { return this._data }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Object literal with accessor setter")

    suite.addTest("setter and getter accessor used in object literal", () => {
        const obj: WithGetterAndSetter = {value: 15}
        arktest.assertEQ(obj.value, 30)
    })

    suite.addTest("setter-only accessor used in object literal", () => {
        const obj: SetterOnly = {data: "hello"}
        arktest.assertEQ(obj.getData(), "hello")
    })

    suite.addTest("object literal with accessor creates correct instance type", () => {
        const obj: WithGetterAndSetter = {value: 10}
        arktest.assertTrue(obj instanceof WithGetterAndSetter)
    })

    return suite.run()
}
