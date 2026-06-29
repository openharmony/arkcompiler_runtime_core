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
const createOuterObj = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createOuterObj');
const createDeepNestedObj = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createDeepNestedObj');
const createBigIntPositive = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createBigIntPositive');
const createBigIntNegative = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createBigIntNegative');
const createBigIntSafeBoundary = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createBigIntSafeBoundary');
const createBigIntMixed = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createBigIntMixed');
const createBigIntInner = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createBigIntInner');
const createBigIntNested = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createBigIntNested');
const createBigIntMultiple = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createBigIntMultiple');
const createBigIntZero = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createBigIntZero');
const createBigIntFloat = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createBigIntFloat');
const createUnicodeNull = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createUnicodeNull');
const createUnicodeControl = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createUnicodeControl');
const createUnicodeMultibyte = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createUnicodeMultibyte');
const createUnicodeString = etsVm.getFunction('Lto_json_stringify/ETSGLOBAL;', 'createUnicodeString');

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

// Test: JSON.stringify(a.b) where a and b are both ETS objects
function testNestedEtsObjectStringify(): void {
    let outer = createOuterObj();
    // Serialize the sub-object a.b directly
    let innerJson = JSON.stringify(outer.inner);
    ASSERT_EQ(innerJson, '{"value":"inner","count":42}');
}

// Test: JSON.stringify(a) where a contains a nested ETS object
function testOuterEtsObjectStringify(): void {
    let outer = createOuterObj();
    let json = JSON.stringify(outer);
    ASSERT_EQ(json, '{"label":"outer","inner":{"value":"inner","count":42}}');
}

// Test: JSON.stringify(a.b.c) deep nesting
function testDeepNestedEtsObjectStringify(): void {
    let deep = createDeepNestedObj();
    // Serialize a.b (child is an OuterObj)
    let childJson = JSON.stringify(deep.child);
    ASSERT_EQ(childJson, '{"label":"outer","inner":{"value":"inner","count":42}}');
    // Serialize a.b.c (child.inner is an InnerObj)
    let innerJson = JSON.stringify(deep.child.inner);
    ASSERT_EQ(innerJson, '{"value":"inner","count":42}');
}

function testBigIntPositiveStringify(): void {
    let obj = createBigIntPositive();
    let json = JSON.stringify(obj);
    ASSERT_EQ(json, '{"bigValue":"12345678901234567890"}');
}

function testBigIntNegativeStringify(): void {
    let obj = createBigIntNegative();
    let json = JSON.stringify(obj);
    ASSERT_EQ(json, '{"bigNeg":"-12345678901234567890"}');
}

function testBigIntSafeBoundaryStringify(): void {
    let obj = createBigIntSafeBoundary();
    let json = JSON.stringify(obj);
    // unsafe (9007199254740992) exceeds MAX_SAFE_INTEGER → quoted
    // safe (9007199254740991) is exactly MAX_SAFE_INTEGER → kept as number
    ASSERT_EQ(json, '{"unsafe":"9007199254740992","safe":9007199254740991}');
}

function testBigIntMixedStringify(): void {
    let obj = createBigIntMixed();
    let json = JSON.stringify(obj);
    ASSERT_EQ(json, '{"big":"12345678901234567890","small":42,"name":"hello"}');
}

function testBigIntNestedStringify(): void {
    let obj = createBigIntNested();
    let json = JSON.stringify(obj);
    ASSERT_EQ(json, '{"outer":{"innerBig":"12345678901234567890"}}');
}

// Test: JSON.stringify(a.b) where b (inner ETS object) contains BigInt
function testBigIntInnerObjectStringify(): void {
    let nested = createBigIntNested();
    let innerJson = JSON.stringify(nested.outer);
    ASSERT_EQ(innerJson, '{"innerBig":"12345678901234567890"}');
}

function testBigIntMultipleStringify(): void {
    let obj = createBigIntMultiple();
    let json = JSON.stringify(obj);
    ASSERT_EQ(json, '{"a":"12345678901234567890","b":"98765432109876543210"}');
}

function testBigIntZeroStringify(): void {
    let obj = createBigIntZero();
    let json = JSON.stringify(obj);
    ASSERT_EQ(json, '{"zero":0}');
}

function testUnicodeNullStringify(): void {
    let obj = createUnicodeNull();
    let json = JSON.stringify(obj);
    ASSERT_EQ(json, '{"value":"\\u0000","value2":"\\u0001","value3":"\\u0002","value4":"đ"}');
}

function testUnicodeControlStringify(): void {
    let obj = createUnicodeControl();
    let json = JSON.stringify(obj);
    ASSERT_EQ(json, '{"lastCtrl":"\\u001f","space":" "}');
}

function testUnicodeMultibyteStringify(): void {
    let obj = createUnicodeMultibyte();
    let json = JSON.stringify(obj);
    ASSERT_EQ(json, '{"cjk":"中","heart":"♥"}');
}

function testUnicodeStringStringify(): void {
    let obj = createUnicodeString();
    let json = JSON.stringify(obj);
    ASSERT_EQ(json, '{"nullInStr":"a\\u0000b","mix":"hello中world"}');
}

function testBigIntFloatStringify(): void {
    let obj = createBigIntFloat();
    let json = JSON.stringify(obj);
    ASSERT_EQ(json, '{"pi":3.141592653589793}');
}

function main(): void {
    testStandardStringify();
    // TODO: issue #34464 to track the problem.
    // testCustomStringify();
    // testInheritanceStringify();
    // testOverridingStringify();
    // testNestedStringify();
    testEnumStringify();
    testEnumInObjectStringify();
    testNestedEtsObjectStringify();
    testOuterEtsObjectStringify();
    testDeepNestedEtsObjectStringify();
    testBigIntPositiveStringify();
    testBigIntNegativeStringify();
    testBigIntSafeBoundaryStringify();
    testBigIntMixedStringify();
    testBigIntNestedStringify();
    testBigIntInnerObjectStringify();
    testBigIntMultipleStringify();
    testBigIntZeroStringify();
    testBigIntFloatStringify();
    testUnicodeNullStringify();
    testUnicodeControlStringify();
    testUnicodeMultibyteStringify();
    testUnicodeStringStringify();
}

main();
