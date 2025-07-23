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

function multiply(a: number, b: number): number {
    return a * b;
}

class MyObject {
    value: number;

    constructor(value: number) {
        this.value = value;
    }

    getDouble(): number {
        return this.value * 2;
    }
}

(globalThis as any).wrapobjTS = new MyObject(55);

function main(): void {
    let etsVm = globalThis.gtest.etsVm;

    nativeSaveRef("NativeGref String");
    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckNativeGrefString")());

    nativeSaveRef(123.456);
    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckNativeGrefNumber")());

    nativeSaveRef({ name: "ArkTS", version: 1 });
    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckNativeGrefObjectField")());

    nativeSaveRef([10, 20, 30]);
    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckNativeGrefArray")());

    nativeSaveRef(multiply);
    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckNativeGrefCallback")());

    nativeSaveRef(null);
    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckNativeGrefNullSafety")());


    nativeSaveRef({
        meta: {
        id: 123,
        tag: "nested",
        inner: { active: true }
        }
    });
    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckNestedObject")());

    nativeSaveRef([
        [1, 2],
        [3, 4],
        [5, 6]
    ]);
    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckNestedArray")());

    const obj = new MyObject(42);
    nativeSaveRef(obj);
    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckObjectWithMethod")());

    const sym = Symbol("secret");
    const objWithSymbol = {
        sym: "hidden",
        visible: "shown"
    };
    nativeSaveRef(objWithSymbol);
    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckSymbolFieldIgnored")());

    nativeSaveRef({ a: 1, b: undefined });
    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckUndefinedField")());

    nativeSaveRef([]);
    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckEmptyArray")());

    nativeSaveRef(new Date("2025-01-01T12:00:00Z"));
    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckDateObject")());
}

(globalThis as any).nativeSaveRef("temp");

const obj = {};

(globalThis as any).nativeWrapRef(obj);
main();
