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

const ClassWithStaticFields = etsVm.getClass('Lstatic_field_accessor/ClassWithStaticFields;');
const ClassWithMixedFields = etsVm.getClass('Lstatic_field_accessor/ClassWithMixedFields;');
const ClassWithOnlyStaticFields = etsVm.getClass('Lstatic_field_accessor/ClassWithOnlyStaticFields;');

const createClassWithStaticFields = etsVm.getFunction('Lstatic_field_accessor/ETSGLOBAL;', 'createClassWithStaticFields');
const createClassWithMixedFields = etsVm.getFunction('Lstatic_field_accessor/ETSGLOBAL;', 'createClassWithMixedFields');
const createClassWithOnlyStaticFields = etsVm.getFunction('Lstatic_field_accessor/ETSGLOBAL;', 'createClassWithOnlyStaticFields');

// Test 1: Verify the JS constructor for a class with static fields is valid (not undefined)
function testClassCtorIsValid(): void {
    ASSERT_TRUE(ClassWithStaticFields !== undefined);
    ASSERT_TRUE(typeof ClassWithStaticFields === 'function');

    ASSERT_TRUE(ClassWithMixedFields !== undefined);
    ASSERT_TRUE(typeof ClassWithMixedFields === 'function');

    ASSERT_TRUE(ClassWithOnlyStaticFields !== undefined);
    ASSERT_TRUE(typeof ClassWithOnlyStaticFields === 'function');
}

// Test 2: Verify static fields are readable on the constructor
function testStaticFieldRead(): void {
    ASSERT_EQ(ClassWithStaticFields.counter, 42);
    ASSERT_EQ(ClassWithStaticFields.label, 'hello');
}

// Test 3: Verify static fields are writable on the constructor
function testStaticFieldWrite(): void {
    ClassWithStaticFields.counter = 99;
    ASSERT_EQ(ClassWithStaticFields.counter, 99);

    ClassWithStaticFields.label = 'world';
    ASSERT_EQ(ClassWithStaticFields.label, 'world');
}

// Test 4: Verify instance creation works for class with static fields
function testInstanceCreation(): void {
    let obj = createClassWithStaticFields();
    ASSERT_TRUE(obj !== undefined);
    ASSERT_EQ(obj.instanceValue, 100);
}

// Test 5: Verify mixed (static + instance) fields all work correctly
function testMixedFields(): void {
    ASSERT_EQ(ClassWithMixedFields.totalCount, 0);
    ASSERT_EQ(ClassWithMixedFields.prefix, 'item');

    let obj = createClassWithMixedFields();
    ASSERT_TRUE(obj !== undefined);
    ASSERT_EQ(obj.name, 'default');
    ASSERT_EQ(obj.value, 10);
}

// Test 6: Verify a class with only static fields works
function testOnlyStaticFields(): void {
    ASSERT_EQ(ClassWithOnlyStaticFields.x, 1);
    ASSERT_EQ(ClassWithOnlyStaticFields.y, 2);
    ASSERT_EQ(ClassWithOnlyStaticFields.z, 3);
}

// Test 7: Verify JSON.stringify works on instances of classes with static fields
function testJsonStringifyWithStaticFields(): void {
    let obj = createClassWithStaticFields();
    let json = JSON.stringify(obj);
    // JSON.stringify should serialize instance fields only (static fields are on the constructor)
    ASSERT_TRUE(json !== undefined);
    ASSERT_TRUE(json.includes('"instanceValue"'));
}

// Test 8: Verify JSON.stringify works on instances of mixed-field classes
function testJsonStringifyMixedFields(): void {
    let obj = createClassWithMixedFields();
    let json = JSON.stringify(obj);
    ASSERT_TRUE(json !== undefined);
    ASSERT_TRUE(json.includes('"name"'));
    ASSERT_TRUE(json.includes('"value"'));
}

function main(): void {
    testClassCtorIsValid();
    testStaticFieldRead();
    testStaticFieldWrite();
    testInstanceCreation();
    testMixedFields();
    testOnlyStaticFields();
    testJsonStringifyWithStaticFields();
    testJsonStringifyMixedFields();
}

main();
