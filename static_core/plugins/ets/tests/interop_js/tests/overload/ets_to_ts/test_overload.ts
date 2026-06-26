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

    ASSERT_TRUE(handler.handlePet(dog) === 'dog', 'Expected "dog" but got: ' + handler.handlePet(dog));
    ASSERT_TRUE(handler.handlePet(cat) === 'cat', 'Expected "cat" but got: ' + handler.handlePet(cat));
}

function testObjectOverloadSpecificVsObject(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createObjectOverloadHandler');
    let createDog = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createDog');

    let handler = createHandler();
    let dog = createDog();

    ASSERT_TRUE(handler.handleSpecific(dog) === 'dog',
        'Expected "dog" (specific overload) but got: ' + handler.handleSpecific(dog));

    try {
        handler.handleSpecific({});
        ASSERT_TRUE(false, 'Expected TypeError for plain JS object');
    } catch (e) {
        ASSERT_TRUE(e.message.includes('not assignable'),
            'Expected "not assignable" error for {} but got: ' + e.message);
    }

    try {
        handler.handleSpecific(null);
        ASSERT_TRUE(false, 'Expected TypeError for null');
    } catch (e) {
        ASSERT_TRUE(e.message.includes('not assignable'),
            'Expected "not assignable" error for null but got: ' + e.message);
    }

    ASSERT_TRUE(handler.handleSpecific(undefined) === 'dog',
        'Expected "dog" for undefined (dispatched to Dog overload) but got: ' + handler.handleSpecific(undefined));
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

function testIntVsAny(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createObjectOverloadPrimitiveHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleIntOrAny(42) === 'int',
        'Expected "int" for integer but got: ' + handler.handleIntOrAny(42));

    ASSERT_TRUE(handler.handleIntOrAny({a: 1}) === 'any',
        'Expected "any" for JS object but got: ' + handler.handleIntOrAny({a: 1}));

    ASSERT_TRUE(handler.handleIntOrAny(null) === 'any',
        'Expected "any" for null but got: ' + handler.handleIntOrAny(null));

    ASSERT_TRUE(handler.handleIntOrAny(undefined) === 'any',
        'Expected "any" for undefined but got: ' + handler.handleIntOrAny(undefined));

    ASSERT_TRUE(handler.handleIntOrAny(true) === 'any',
        'Expected "any" for boolean but got: ' + handler.handleIntOrAny(true));

    ASSERT_TRUE(handler.handleIntOrAny('hello') === 'any',
        'Expected "any" for string but got: ' + handler.handleIntOrAny('hello'));
}

function testStringVsAny(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createObjectOverloadPrimitiveHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleStringOrAny('hello') === 'string',
        'Expected "string" for string but got: ' + handler.handleStringOrAny('hello'));

    ASSERT_TRUE(handler.handleStringOrAny(42) === 'any',
        'Expected "any" for number but got: ' + handler.handleStringOrAny(42));

    ASSERT_TRUE(handler.handleStringOrAny(null) === 'any',
        'Expected "any" for null but got: ' + handler.handleStringOrAny(null));

    ASSERT_TRUE(handler.handleStringOrAny(undefined) === 'any',
        'Expected "any" for undefined but got: ' + handler.handleStringOrAny(undefined));

    ASSERT_TRUE(handler.handleStringOrAny(true) === 'any',
        'Expected "any" for boolean but got: ' + handler.handleStringOrAny(true));
}

function testAnyVsInt(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createObjectOverloadPrimitiveHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleAnyOrInt({a: 1}) === 'any',
        'Expected "any" for JS object but got: ' + handler.handleAnyOrInt({a: 1}));

    ASSERT_TRUE(handler.handleAnyOrInt(null) === 'any',
        'Expected "any" for null but got: ' + handler.handleAnyOrInt(null));

    ASSERT_TRUE(handler.handleAnyOrInt(undefined) === 'any',
        'Expected "any" for undefined but got: ' + handler.handleAnyOrInt(undefined));
}

function testDogVsAny(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createObjectOverloadPrimitiveHandler');
    let createDog = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createDog');
    let handler = createHandler();
    let dog = createDog();

    ASSERT_TRUE(handler.handleDogOrAny(dog) === 'dog',
        'Expected "dog" for Dog instance but got: ' + handler.handleDogOrAny(dog));

    try {
        handler.handleDogOrAny({a: 1});
        ASSERT_TRUE(false, 'Expected TypeError for plain JS object');
    } catch (e) {
        ASSERT_TRUE(e.message.includes('not assignable'),
            'Expected "not assignable" error for {} but got: ' + e.message);
    }

    try {
        handler.handleDogOrAny(null);
        ASSERT_TRUE(false, 'Expected TypeError for null');
    } catch (e) {
        ASSERT_TRUE(e.message.includes('not assignable'),
            'Expected "not assignable" error for null but got: ' + e.message);
    }
}

function testCheckStringVsAny(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleStringOrAny('hello') === 'string',
        'Expected "string" for string but got: ' + handler.handleStringOrAny('hello'));

    ASSERT_TRUE(handler.handleStringOrAny(42) === 'any',
        'Expected "any" for number but got: ' + handler.handleStringOrAny(42));

    ASSERT_TRUE(handler.handleStringOrAny(null) === 'any',
        'Expected "any" for null but got: ' + handler.handleStringOrAny(null));

    ASSERT_TRUE(handler.handleStringOrAny(undefined) === 'any',
        'Expected "any" for undefined but got: ' + handler.handleStringOrAny(undefined));
}

function testCheckBigIntVsAny(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleBigIntOrAny(123n) === 'bigint',
        'Expected "bigint" for bigint but got: ' + handler.handleBigIntOrAny(123n));

    ASSERT_TRUE(handler.handleBigIntOrAny(42) === 'any',
        'Expected "any" for number but got: ' + handler.handleBigIntOrAny(42));

    ASSERT_TRUE(handler.handleBigIntOrAny(null) === 'any',
        'Expected "any" for null but got: ' + handler.handleBigIntOrAny(null));
}

function testFunctionVsInt(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleFnOrAny(42) === 'int',
        'Expected "int" for number but got: ' + handler.handleFnOrAny(42));

    ASSERT_TRUE(handler.handleFnOrAny(function() {}) === 'function',
        'Expected "function" for function but got: ' + handler.handleFnOrAny(function() {}));

    ASSERT_TRUE(handler.handleFnOrAny({a: 1}) === 'function',
        'Expected "function" for JS object but got: ' + handler.handleFnOrAny({a: 1}));

    ASSERT_TRUE(handler.handleFnOrAny(null) === 'function',
        'Expected "function" for null but got: ' + handler.handleFnOrAny(null));
}

function testBoolVsAny(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleBoolOrAny(true) === 'boolean',
        'Expected "boolean" for boolean but got: ' + handler.handleBoolOrAny(true));

    ASSERT_TRUE(handler.handleBoolOrAny(false) === 'boolean',
        'Expected "boolean" for false but got: ' + handler.handleBoolOrAny(false));

    ASSERT_TRUE(handler.handleBoolOrAny(42) === 'any',
        'Expected "any" for number but got: ' + handler.handleBoolOrAny(42));

    ASSERT_TRUE(handler.handleBoolOrAny(null) === 'any',
        'Expected "any" for null but got: ' + handler.handleBoolOrAny(null));

    ASSERT_TRUE(handler.handleBoolOrAny({a: 1}) === 'any',
        'Expected "any" for JS object but got: ' + handler.handleBoolOrAny({a: 1}));
}

function testDoubleVsAny(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleDoubleOrAny(1.5) === 'double',
        'Expected "double" for double but got: ' + handler.handleDoubleOrAny(1.5));

    ASSERT_TRUE(handler.handleDoubleOrAny(1e40) === 'double',
        'Expected "double" for large double but got: ' + handler.handleDoubleOrAny(1e40));

    ASSERT_TRUE(handler.handleDoubleOrAny(42) === 'double',
        'Expected "double" for integer number but got: ' + handler.handleDoubleOrAny(42));

    ASSERT_TRUE(handler.handleDoubleOrAny(null) === 'any',
        'Expected "any" for null but got: ' + handler.handleDoubleOrAny(null));

    ASSERT_TRUE(handler.handleDoubleOrAny({a: 1}) === 'any',
        'Expected "any" for JS object but got: ' + handler.handleDoubleOrAny({a: 1}));
}

function testMultiArgOverload(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let handler = createHandler();

    ASSERT_TRUE(handler.handleTwoArgs('hello', 42) === 'string int',
        'Expected "string int" for (string, number) but got: ' + handler.handleTwoArgs('hello', 42));

    ASSERT_TRUE(handler.handleTwoArgs(42, 42) === 'any any',
        'Expected "any any" for (number, number) but got: ' + handler.handleTwoArgs(42, 42));

    ASSERT_TRUE(handler.handleTwoArgs({}, null) === 'any any',
        'Expected "any any" for (object, null) but got: ' + handler.handleTwoArgs({}, null));

    ASSERT_TRUE(handler.handleTwoArgs('hello', {}) === 'any any',
        'Expected "any any" for (string, object) but got: ' + handler.handleTwoArgs('hello', {}));
}

function testInheritanceChainWithAny(): void {
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

    try {
        handler.handleDogOrAnimalOrAny({});
        ASSERT_TRUE(false, 'Expected TypeError for plain JS object');
    } catch (e) {
        ASSERT_TRUE(e.message.includes('not assignable'),
            'Expected "not assignable" error for {} but got: ' + e.message);
    }

    try {
        handler.handleDogOrAnimalOrAny(null);
        ASSERT_TRUE(false, 'Expected TypeError for null');
    } catch (e) {
        ASSERT_TRUE(e.message.includes('not assignable'),
            'Expected "not assignable" error for null but got: ' + e.message);
    }

    ASSERT_TRUE(handler.handleDogOrAnimalOrAny(undefined) === 'dog',
        'Expected "dog" for undefined (dispatched to Dog overload) but got: ' + handler.handleDogOrAnimalOrAny(undefined));
}

function testThreeArgSpecificVsAny(): void {
    let createHandler = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createCheckArgMatchObjectHandler');
    let createDog = etsVm.getFunction('Ltest_overload/ETSGLOBAL;', 'createDog');
    let handler = createHandler();
    let dog = createDog();

    ASSERT_TRUE(handler.handleThreeSpecific('hello', 42, dog) === 'string int dog',
        'Expected "string int dog" for (string, int, Dog) but got: ' + handler.handleThreeSpecific('hello', 42, dog));

    ASSERT_TRUE(handler.handleThreeSpecific(42, 42, dog) === 'any any any',
        'Expected "any any any" for (number, int, Dog) but got: ' + handler.handleThreeSpecific(42, 42, dog));

    try {
        handler.handleThreeSpecific('hello', 42, {});
        ASSERT_TRUE(false, 'Expected TypeError for {} as Dog arg');
    } catch (e) {
        ASSERT_TRUE(e.message.includes('not assignable'),
            'Expected "not assignable" error for {} as Dog but got: ' + e.message);
    }

    ASSERT_TRUE(handler.handleThreeSpecific('hello', {}, dog) === 'any any any',
        'Expected "any any any" for (string, object, Dog) but got: ' + handler.handleThreeSpecific('hello', {}, dog));

    ASSERT_TRUE(handler.handleThreeSpecific({}, null, undefined) === 'any any any',
        'Expected "any any any" for (object, null, undefined) but got: ' + handler.handleThreeSpecific({}, null, undefined));
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

    ASSERT_TRUE(handler.handleThreeInherit(animal, animal, animal) === 'animal animal any',
        'Expected "animal animal any" for (Animal, Animal, Animal) but got: ' + handler.handleThreeInherit(animal, animal, animal));

    ASSERT_TRUE(handler.handleThreeInherit(animal, animal, null) === 'animal animal any',
        'Expected "animal animal any" for (Animal, Animal, null) but got: ' + handler.handleThreeInherit(animal, animal, null));

    try {
        handler.handleThreeInherit({}, {}, {});
        ASSERT_TRUE(false, 'Expected TypeError for {} args');
    } catch (e) {
        ASSERT_TRUE(e.message.includes('not assignable'),
            'Expected "not assignable" error for {} args but got: ' + e.message);
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
    testIntVsAny();
    testStringVsAny();
    testAnyVsInt();
    testDogVsAny();
    testCheckStringVsAny();
    testCheckBigIntVsAny();
    testFunctionVsInt();
    testBoolVsAny();
    testDoubleVsAny();
    testMultiArgOverload();
    testInheritanceChainWithAny();
    testThreeArgSpecificVsAny();
    testThreeArgInheritanceChain();
}

function main(): void {
    testAll();
}

main();
