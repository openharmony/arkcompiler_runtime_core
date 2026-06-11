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
const checkArrRefEquality = etsVm.getFunction('Lets_invalid_sharedref/ETSGLOBAL;', 'checkArrRefEquality');
const checkTwoNotEqual = etsVm.getFunction('Lets_invalid_sharedref/ETSGLOBAL;', 'checkTwoNotEqual');
const checkFuncRef = etsVm.getFunction('Lets_invalid_sharedref/ETSGLOBAL;', 'checkFuncRef');
const checkAreEqual = etsVm.getFunction('Lets_invalid_sharedref/ETSGLOBAL;', 'checkAreEqual');

// Export JS object for ETS side to access via module.getProperty
export let sharedObj = etsVm.STValue.newSTArray();
sharedObj.push('foo', 1, true);
export let o = {a: 1};

// Test 1: SharedReference identity - same STArray passed twice
let jsObj = etsVm.STValue.newSTArray();
jsObj.push('foo', 1, true);
ASSERT_TRUE(!checkArrRefEquality(jsObj));
ASSERT_TRUE(checkArrRefEquality(jsObj));

// Test 2: different objects should NOT be === equal
let jsObj1 = etsVm.STValue.newSTArray();
jsObj1.push('a');
let jsObj2 = etsVm.STValue.newSTArray();
jsObj2.push('b');
ASSERT_TRUE(checkTwoNotEqual(jsObj1, jsObj2));

// Test 3: SharedReference identity for JS functions
let fn = (): void => { print('hello'); };
ASSERT_TRUE(!checkFuncRef(fn));
ASSERT_TRUE(checkFuncRef(fn));

// Test 4: ESValue.areEqual / areStrictlyEqual via module.getProperty
ASSERT_TRUE(checkAreEqual());
