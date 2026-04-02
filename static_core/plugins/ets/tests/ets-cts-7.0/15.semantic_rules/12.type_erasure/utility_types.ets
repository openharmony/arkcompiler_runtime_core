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
    Type erasure for utility types. Effective type mapping for
    NonNullable<T>, Required<T>, Readonly<T>, Partial<T>, Record<K,V>, ReturnType<F>.
---*/

class A<T> {
    field: T
    constructor(value: T) {
        this.field = value
    }
}

interface Opts {
    name?: string
    value?: number
}

type NumRet = ReturnType<() => number>
type GetStringType = () => string
type StrRet = ReturnType<GetStringType>

function main(): int {
    let suite = new arktest.ArkTestsuite("UtilityTypesTypeErasure");

    suite.addTest("NonNullable cast preserves type", () => {
        let obj: string | null = "hello"
        let x = ((): Object => obj)() as NonNullable<string | null>
        arktest.assertEQ(x, "hello")
    });

    suite.addTest("NonNullable removes null from union", () => {
        let val: NonNullable<string | null> = "world"
        arktest.assertEQ(val, "world")
    });

    suite.addTest("Required type cast", () => {
        let obj: Required<Opts> = { name: "test", value: 36 }
        arktest.assertEQ(obj.name, "test")
        arktest.assertEQ(obj.value, 36)
    });

    suite.addTest("Readonly type cast", () => {
        let obj: Readonly<A<string>> = new A<string>("readonly")
        arktest.assertEQ(obj.field, "readonly")
    });

    suite.addTest("Record type basic usage", () => {
        let rec: Record<string, number> = { "a": 1.0, "b": 2.0 }
        arktest.assertEQ(rec["a"], 1.0)
        arktest.assertEQ(rec["b"], 2.0)
    });

    suite.addTest("Record type cast from Any", () => {
        let obj: Record<string, number> = { "x": 37.5 }
        let x = ((): Any => obj)() as Record<string, number>
        arktest.assertEQ(x["x"], 37.5)
    });

    suite.addTest("Partial makes all fields optional", () => {
        let partial: Partial<Opts> = { name: "test" }
        arktest.assertEQ(partial.name, "test")
        arktest.assertEQ(partial.value, undefined)
    });

    suite.addTest("Partial with all fields set", () => {
        let partial: Partial<Opts> = { name: "full", value: 50 }
        arktest.assertEQ(partial.name, "full")
        arktest.assertEQ(partial.value, 50)
    });

    suite.addTest("Partial with no fields set", () => {
        let partial: Partial<Opts> = {}
        arktest.assertEQ(partial.name, undefined)
        arktest.assertEQ(partial.value, undefined)
    });

    suite.addTest("ReturnType extracts function return type", () => {
        let val: NumRet = 38.0
        arktest.assertEQ(val, 38.0)
    });

    suite.addTest("ReturnType with named function", () => {
        let val: StrRet = "result"
        arktest.assertEQ(val, "result")
    });

    return suite.run();
}
