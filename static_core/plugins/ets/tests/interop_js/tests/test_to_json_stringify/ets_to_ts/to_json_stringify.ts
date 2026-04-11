/**
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

const etsVm = globalThis.gtest.etsVm;

const StandardClass = etsVm.getClass('Lto_json_stringify/StandardClass;');
const createStandard = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createStandard');

const CustomClass = etsVm.getClass('Lto_json_stringify/CustomClass;');
const createCustom = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createCustom');

const createBase = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createBase');
const createDerived = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createDerived');
const createOverriding = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createOverriding');
const createNested = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createNested');
const createColorRed = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createColorRed');
const createStringColorGreen = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createStringColorGreen');
const createEnumContainer = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createEnumContainer');

function testStandardStringify(): void {
    let obj = createStandard();
    let json = JSON.stringify(obj);
    // Standard serialization should include fields
    ASSERT_TRUE(json === '{"name":"Alice","age":30}');
}

function testCustomStringify(): void {
    let obj = createCustom();
    let json = JSON.stringify(obj);
    // Custom serialization should return the parsed object from custom toJSON string
    ASSERT_TRUE(json === '"{\\"custom\\":\\"json_works\\"}"');
    ASSERT_TRUE(Object.getOwnPropertyNames(obj).length === 1);
}

function testInheritanceStringify(): void {
    let base = createBase();
    let derived = createDerived();

    ASSERT_EQ(JSON.stringify(base), '"{\\"base\\":true}"');
    ASSERT_EQ(JSON.stringify(derived), '"{\\"base\\":true}"');

    ASSERT_TRUE(Object.getOwnPropertyNames(base).length === 1);
    ASSERT_TRUE(Object.getOwnPropertyNames(derived).length === 1);
}

function testOverridingStringify(): void {
    let obj = createOverriding();
    ASSERT_EQ(JSON.stringify(obj), '"{\\"overridden\\":true}"');
    ASSERT_TRUE(Object.getOwnPropertyNames(obj).length === 1);
}

function testNestedStringify(): void {
    let obj = createNested();
    let json = JSON.stringify(obj);
    let expected = '"{\\"nested\\":{\\"deep\\":true},\\"array\\":[1,2,3]}"';
    ASSERT_EQ(json, expected);
    ASSERT_TRUE(Object.getOwnPropertyNames(obj).length === 1);
}

function testEnumStringify(): void {
    let red = createColorRed();
    ASSERT_EQ(JSON.stringify(red), '0');

    let green = createStringColorGreen();
    ASSERT_EQ(JSON.stringify(green), '"green"');
}

function testEnumInObjectStringify(): void {
    let obj = createEnumContainer();
    let json = JSON.stringify(obj);
    ASSERT_EQ(json, '{"color":2,"stringColor":"red"}');
}

function main(): void {
    testStandardStringify();
    testCustomStringify();
    testInheritanceStringify();
    testOverridingStringify();
    testNestedStringify();
    testEnumStringify();
    testEnumInObjectStringify();
}

main();
