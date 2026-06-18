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

function testAll(): void {
    testExtendsOverloadConflict();
    testStaInsOverload();
    testCountOverload();
    testTypeOverload();
    testComplexOverload();
    testGenericOverload();
}

function main(): void {
    testAll();
}

main();
