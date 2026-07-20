/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const etsVm = globalThis.gtest.etsVm;

function testExtendsOverloadConflict(): void {
    etsVm.getClass('Ltest_overload/CSSLogicRule;');
}

function testStaInsOverload(): void {
    let cssLogicRuleIns = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCSSLogicRule');
    cssLogicRuleIns();
}
function testCountOverload(): void {
    let countOverloadRule = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCountOverloadRule');
    ASSERT_TRUE(countOverloadRule().mergeRule(1) === 'one');
    ASSERT_TRUE(countOverloadRule().mergeRule(200, 300) === 'two');
    ASSERT_TRUE(countOverloadRule().mergeRule(1, 2, 3) === 'three');
}

function testTypeOverload(): void {
    let typeOverloadRuleClass = etsVm.getClass('Ltest_overload/TypeOverloadRule;');
    typeOverloadRuleClass.mergeRuleStatic(1);
    typeOverloadRuleClass.mergeRuleStatic(200);
    typeOverloadRuleClass.mergeRuleStatic(40000);
    typeOverloadRuleClass.mergeRuleStatic(3000000000);
    typeOverloadRuleClass.mergeRuleStatic('hello');
    typeOverloadRuleClass.mergeRuleStatic(1.1);
    typeOverloadRuleClass.mergeRuleStatic(1e40);
    typeOverloadRuleClass.mergeRuleStatic(true);
    typeOverloadRuleClass.mergeRuleStatic(123n);
    try {
        typeOverloadRuleClass.mergeRuleStatic([1, 2]);
    } catch (e) {
        ASSERT_TRUE(e.message.includes('is not supported'));
    }
    try {
        typeOverloadRuleClass.mergeRuleStatic(new Map([['a', 1]]));
    } catch (e) {
        ASSERT_TRUE(e.message.includes('is not supported'));
    }
    try {
        typeOverloadRuleClass.mergeRuleStatic(new Set([1, 2]));
    } catch (e) {
        ASSERT_TRUE(e.message.includes('is not supported'));
    }

    let typeOverloadRule = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createTypeOverloadRule');
    ASSERT_TRUE(typeOverloadRule().mergeRule(1) === 'byte');
    ASSERT_TRUE(typeOverloadRule().mergeRule(200) === 'short');
    ASSERT_TRUE(typeOverloadRule().mergeRule(40000) === 'int');
    ASSERT_TRUE(typeOverloadRule().mergeRule(3000000000) === 'long');
    ASSERT_TRUE(typeOverloadRule().mergeRule('hello') === 'string');
    ASSERT_TRUE(typeOverloadRule().mergeRule(1.1) === 'float');
    ASSERT_TRUE(typeOverloadRule().mergeRule(1e40) === 'double');
    ASSERT_TRUE(typeOverloadRule().mergeRule(true) === 'boolean');
    ASSERT_TRUE(typeOverloadRule().mergeRule(123n) === 'bigint');
    try {
        typeOverloadRule().mergeRule([1, 2]);
    } catch (e) {
        ASSERT_TRUE(e.message.includes('is not supported'));
    }
    try {
        typeOverloadRule().mergeRule(new Map([['a', 1]]));
    } catch (e) {
        ASSERT_TRUE(e.message.includes('is not supported'));
    }
    try {
        typeOverloadRule().mergeRule(new Set([1, 2]));
    } catch (e) {
        ASSERT_TRUE(e.message.includes('is not supported'));
    }
}

function testComplexOverload(): void {
    let complexOverloadRule = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createComplexOverloadRule');
    ASSERT_TRUE(complexOverloadRule().mergeRule() === 'rest int');
    ASSERT_TRUE(complexOverloadRule().mergeRule('hello') === 'string');
    ASSERT_TRUE(complexOverloadRule().mergeRule('hello', 'world') === 'two string');
    ASSERT_TRUE(complexOverloadRule().mergeRule('hello', 1) === 'string byte');
    ASSERT_TRUE(complexOverloadRule().mergeRule('hello', 200) === 'string short');
    ASSERT_TRUE(complexOverloadRule().mergeRule('hello', 40000) === 'string int');
    ASSERT_TRUE(complexOverloadRule().mergeRule('hello', 3000000000) === 'string long');
    ASSERT_TRUE(complexOverloadRule().mergeRule('hello', 1.12) === 'string float');
    ASSERT_TRUE(complexOverloadRule().mergeRule('hello', 1e40) === 'string double');
}

function testGenericOverload(): void {
    let genericOverloadRule = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createGenericOverloadRule');
    ASSERT_TRUE(genericOverloadRule().mergeRule(123n) === 'generic');
    ASSERT_TRUE(genericOverloadRule().mergeRule(true) === 'generic');
    ASSERT_TRUE(genericOverloadRule().mergeRule(1) === 'generic');
    ASSERT_TRUE(genericOverloadRule().mergeRule('hello') === 'generic');
}

function testObjectOverloadMutuallyExclusive(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createObjectOverloadHandler');
    let createDog = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createDog');
    let createCat = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCat');

    let handler = createHandler();
    let dog = createDog();
    let cat = createCat();

    ASSERT_TRUE(handler.handlePet(dog) === 'pet', 'Expected "pet" but got: ' + handler.handlePet(dog));
    ASSERT_TRUE(handler.handlePet(cat) === 'pet', 'Expected "pet" but got: ' + handler.handlePet(cat));
}

function testObjectOverloadSpecificVsObject(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createObjectOverloadHandler');
    let createDog = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createDog');

    let handler = createHandler();
    let dog = createDog();

    ASSERT_TRUE(handler.handleSpecific(dog) === 'dog',
        'Expected "dog" (specific overload) but got: ' + handler.handleSpecific(dog));

    ASSERT_TRUE(handler.handleSpecific({}) === 'object',
        'Expected "object" for plain JS object but got: ' + handler.handleSpecific({}));

    ASSERT_TRUE(handler.handleSpecific(null) === 'null',
        'Expected "null" for null (dispatched to null overload) but got: ' + handler.handleSpecific(null));
}

function testObjectOverloadInheritance(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createObjectOverloadHandler');
    let createDog = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createDog');
    let createAnimal = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createAnimal');

    let handler = createHandler();
    let dog = createDog();
    let animal = createAnimal();

    ASSERT_TRUE(handler.handleAnimal(dog) === 'dog',
        'Expected "dog" for Dog instance but got: ' + handler.handleAnimal(dog));
    ASSERT_TRUE(handler.handleAnimal(animal) === 'animal',
        'Expected "animal" for Animal instance but got: ' + handler.handleAnimal(animal));
}

function testIntVsObject(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createObjectOverloadPrimitiveHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleIntOrAny(42) === 'int',
        'Expected "int" for integer but got: ' + handler.handleIntOrAny(42));

    ASSERT_TRUE(handler.handleIntOrAny({a: 1}) === 'object',
        'Expected "object" for JS object but got: ' + handler.handleIntOrAny({a: 1}));

    ASSERT_TRUE(handler.handleIntOrAny(null) === 'null',
        'Expected "null" for null but got: ' + handler.handleIntOrAny(null));

    // JS undefined matches ETS Object through IsObjectClass check
    ASSERT_TRUE(handler.handleIntOrAny(undefined) === 'object',
        'Expected "object" for undefined but got: ' + handler.handleIntOrAny(undefined));

    // JS boolean matches ETS Object through IsObjectClass check, conversion wraps as Object
    ASSERT_TRUE(handler.handleIntOrAny(true) === 'object',
        'Expected "object" for boolean but got: ' + handler.handleIntOrAny(true));

    // JS string matches ETS Object through IsObjectClass check (ETS String extends Object)
    ASSERT_TRUE(handler.handleIntOrAny('hello') === 'object',
        'Expected "object" for string but got: ' + handler.handleIntOrAny('hello'));
}

function testStringVsObject(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createObjectOverloadPrimitiveHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleStringOrAny('hello') === 'string',
        'Expected "string" for string but got: ' + handler.handleStringOrAny('hello'));

    // JS number matches ETS Object through IsObjectClass check
    ASSERT_TRUE(handler.handleStringOrAny(42) === 'object',
        'Expected "object" for number but got: ' + handler.handleStringOrAny(42));

    ASSERT_TRUE(handler.handleStringOrAny(null) === 'null',
        'Expected "null" for null but got: ' + handler.handleStringOrAny(null));

    // JS boolean matches ETS Object through IsObjectClass check
    ASSERT_TRUE(handler.handleStringOrAny(true) === 'object',
        'Expected "object" for boolean but got: ' + handler.handleStringOrAny(true));
}

function testObjectVsInt(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createObjectOverloadPrimitiveHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleAnyOrInt({a: 1}) === 'object',
        'Expected "object" for JS object but got: ' + handler.handleAnyOrInt({a: 1}));

    ASSERT_TRUE(handler.handleAnyOrInt(null) === 'null',
        'Expected "null" for null but got: ' + handler.handleAnyOrInt(null));
}

function testDogVsObject(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createObjectOverloadPrimitiveHandler');
    let createDog = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createDog');
    let handler = createHandler();
    let dog = createDog();

    ASSERT_TRUE(handler.handleDogOrAny(dog) === 'dog',
        'Expected "dog" for Dog instance but got: ' + handler.handleDogOrAny(dog));

    ASSERT_TRUE(handler.handleDogOrAny({a: 1}) === 'object',
        'Expected "object" for plain JS object but got: ' + handler.handleDogOrAny({a: 1}));

    ASSERT_TRUE(handler.handleDogOrAny(null) === 'null',
        'Expected "null" for null instance but got: ' + handler.handleDogOrAny(null));
}

function testCheckStringVsObject(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleStringOrAny('hello') === 'string',
        'Expected "string" for string but got: ' + handler.handleStringOrAny('hello'));

    // JS number matches ETS Object through IsObjectClass check
    ASSERT_TRUE(handler.handleStringOrAny(42) === 'object',
        'Expected "object" for number but got: ' + handler.handleStringOrAny(42));
}

function testCheckBigIntVsObject(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleBigIntOrAny(123n) === 'bigint',
        'Expected "bigint" for bigint but got: ' + handler.handleBigIntOrAny(123n));

    // JS number matches ETS Object through IsObjectClass check
    ASSERT_TRUE(handler.handleBigIntOrAny(42) === 'object',
        'Expected "object" for number but got: ' + handler.handleBigIntOrAny(42));
}

function testFunctionVsObject(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleFnOrAny(42) === 'int',
        'Expected "int" for number but got: ' + handler.handleFnOrAny(42));

    ASSERT_TRUE(handler.handleFnOrAny(function() {}) === 'function',
        'Expected "function" for function but got: ' + handler.handleFnOrAny(function() {}));

    ASSERT_TRUE(handler.handleFnOrAny({a: 1}) === 'function',
        'Expected "function" for JS object but got: ' + handler.handleFnOrAny({a: 1}));
}

function testBoolVsObject(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleBoolOrAny(true) === 'boolean',
        'Expected "boolean" for boolean but got: ' + handler.handleBoolOrAny(true));

    ASSERT_TRUE(handler.handleBoolOrAny(false) === 'boolean',
        'Expected "boolean" for false but got: ' + handler.handleBoolOrAny(false));

    // JS number matches ETS Object through IsObjectClass check
    ASSERT_TRUE(handler.handleBoolOrAny(42) === 'object',
        'Expected "object" for number but got: ' + handler.handleBoolOrAny(42));

    ASSERT_TRUE(handler.handleBoolOrAny({a: 1}) === 'object',
        'Expected "object" for JS object but got: ' + handler.handleBoolOrAny({a: 1}));
}

function testDoubleVsObject(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleDoubleOrAny(1.5) === 'double',
        'Expected "double" for double but got: ' + handler.handleDoubleOrAny(1.5));

    ASSERT_TRUE(handler.handleDoubleOrAny(1e40) === 'double',
        'Expected "double" for large double but got: ' + handler.handleDoubleOrAny(1e40));

    ASSERT_TRUE(handler.handleDoubleOrAny(42) === 'double',
        'Expected "double" for integer number but got: ' + handler.handleDoubleOrAny(42));

    ASSERT_TRUE(handler.handleDoubleOrAny({a: 1}) === 'object',
        'Expected "object" for JS object but got: ' + handler.handleDoubleOrAny({a: 1}));
}

function testMultiArgOverload(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleTwoArgs('hello', 42) === 'string int',
        'Expected "string int" for (string, number) but got: ' + handler.handleTwoArgs('hello', 42));

    ASSERT_TRUE(handler.handleTwoArgs({a: 1}, {a: 1}) === 'object object',
        'Expected "object object" for (object, object) but got: ' + handler.handleTwoArgs({a: 1}, {a: 1}));

    ASSERT_TRUE(handler.handleTwoArgs({}, null) === 'object null',
        'Expected "object null" for (object, null) but got: ' + handler.handleTwoArgs({}, null));

    ASSERT_TRUE(handler.handleTwoArgs('hello', {}) === 'object object',
        'Expected "object object" for (string, object) but got: ' + handler.handleTwoArgs('hello', {}));
}

function testInheritanceChainWithObject(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let createDog = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createDog');
    let createAnimal = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createAnimal');
    let handler = createHandler();
    let dog = createDog();
    let animal = createAnimal();

    ASSERT_TRUE(handler.handleDogOrAnimalOrAny(dog) === 'dog',
        'Expected "dog" for Dog instance but got: ' + handler.handleDogOrAnimalOrAny(dog));

    ASSERT_TRUE(handler.handleDogOrAnimalOrAny(animal) === 'animal',
        'Expected "animal" for Animal instance but got: ' + handler.handleDogOrAnimalOrAny(animal));

    ASSERT_TRUE(handler.handleDogOrAnimalOrAny({}) === 'object',
        'Expected "object" for plain JS object but got: ' + handler.handleDogOrAnimalOrAny({}));
}

function testThreeArgSpecificVsObject(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let createDog = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createDog');
    let handler = createHandler();
    let dog = createDog();

    ASSERT_TRUE(handler.handleThreeSpecific('hello', 42, dog) === 'string int dog',
        'Expected "string int dog" for (string, int, Dog) but got: ' + handler.handleThreeSpecific('hello', 42, dog));

    ASSERT_TRUE(handler.handleThreeSpecific({a: 1}, {a: 1}, {a: 1}) === 'object object object',
        'Expected "object object object" for (object, object, object) but got: ' + handler.handleThreeSpecific({a: 1}, {a: 1}, {a: 1}));

    ASSERT_TRUE(handler.handleThreeSpecific('hello', 42, {}) === 'object object object',
        'Expected "object object object" for (string, int, {}) but got: ' + handler.handleThreeSpecific('hello', 42, {}));

    ASSERT_TRUE(handler.handleThreeSpecific('hello', {}, dog) === 'object object object',
        'Expected "object object object" for (string, object, Dog) but got: ' + handler.handleThreeSpecific('hello', {}, dog));

    ASSERT_TRUE(handler.handleThreeSpecific({}, undefined, undefined) === 'object object object',
        'Expected "object object object" for (object, undefined, undefined) but got: ' + handler.handleThreeSpecific({}, undefined, undefined));
}

function testThreeArgInheritanceChain(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let createDog = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createDog');
    let createAnimal = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createAnimal');
    let handler = createHandler();
    let dog = createDog();
    let animal = createAnimal();

    ASSERT_TRUE(handler.handleThreeInherit(dog, dog, dog) === 'dog dog dog',
        'Expected "dog dog dog" for (Dog, Dog, Dog) but got: ' + handler.handleThreeInherit(dog, dog, dog));

    ASSERT_TRUE(handler.handleThreeInherit(animal, animal, animal) === 'animal animal object',
        'Expected "animal animal object" for (Animal, Animal, Animal) but got: ' + handler.handleThreeInherit(animal, animal, animal));

    ASSERT_TRUE(handler.handleThreeInherit({}, {}, {}) === 'object object object',
        'Expected "object object object" for {} args but got: ' + handler.handleThreeInherit({}, {}, {}));
}

function testHandleNull(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchNullHandler');
    let createDog = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createDog');
    let handler = createHandler();
    let dog = createDog();

    ASSERT_TRUE(handler.handleFirstNull(dog) === 'object',
        'Expected "object" for (Dog) but got: ' + handler.handleFirstNull(dog));

    ASSERT_TRUE(handler.handleFirstNull(null) === 'null',
        'Expected "null" for (null) but got: ' + handler.handleFirstNull(null));

    ASSERT_TRUE(handler.handleFirstNull('string') === 'string',
        'Expected "string" for (string) but got: ' + handler.handleFirstNull(dog));

    ASSERT_TRUE(handler.handleThirdNull(null) === 'null',
        'Expected "null" for (null) but got: ' + handler.handleThirdNull(null));

    ASSERT_TRUE(handler.handleThirdNull(dog) === 'object',
        'Expected "object" for (Dog) but got: ' + handler.handleThirdNull(dog));

    ASSERT_TRUE(handler.handleThirdNull('string') === 'string',
        'Expected "string" for (string) but got: ' + handler.handleThirdNull(dog));

    ASSERT_TRUE(handler.handleUndefinedNull(null) === 'null',
        'Expected "null" for (null) but got: ' + handler.handleUndefinedNull(null));

    ASSERT_TRUE(handler.handleUndefinedNull(undefined) === 'undefined',
        'Expected "undefined" for (undefined) but got: ' + handler.handleUndefinedNull(dog));

    ASSERT_TRUE(handler.handleUndefinedNull('string') === 'string',
        'Expected "string" for (string) but got: ' + handler.handleUndefinedNull(dog));

    try {
        handler.handleNoExpectNull('string');
        ASSERT_TRUE(false, 'Expected TypeError for {} args');
    } catch (e) {
        ASSERT_TRUE(e.message.includes('expected'),
            'Expected "not assignable" error for string arg but got: ' + e.message);
    }
}

function testAll(): void {
    testExtendsOverloadConflict();
    testStaInsOverload();
    testCountOverload();
    testTypeOverload();
    testComplexOverload();
    testGenericOverload();
    testObjectOverloadMutuallyExclusive();
    testObjectOverloadSpecificVsObject();
    testObjectOverloadInheritance();
    testIntVsObject();
    testStringVsObject();
    testObjectVsInt();
    testDogVsObject();
    testCheckStringVsObject();
    testCheckBigIntVsObject();
    testFunctionVsObject();
    testBoolVsObject();
    testDoubleVsObject();
    testMultiArgOverload();
    testInheritanceChainWithObject();
    testThreeArgSpecificVsObject();
    testThreeArgInheritanceChain();
    testHandleNull();
}

function main(): void {
    testAll();
}

main();
