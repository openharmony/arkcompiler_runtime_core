'use strict';
/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
Object.defineProperty(exports, '__esModule', { value: true });
exports.returnThisTestClassInstance = 
    exports.ReturnThisTestClass = 
    exports.testFuncReturnStr = 
    exports.getThisAsAsyncFunction = 
    exports.getThisAsFunction = 
    exports.getThis = 
    exports.TestClass = 
        void 0;
const TEST_ARRAY = [1, 2, 3, 4, 5];
const TEST_STRING = 'Test';
const TEST_NUMBER = 100;
const TEST_NULL = null;
const TEST_MAP = new Map([['TEST', 'test']]);
const TEST_TUPLE = ['Test', 1];
const TEST_OBJ = true;
class TestClass {
    constructor() {
        this.testString = TEST_STRING;
        this.testNumber = TEST_NUMBER;
    }
    getThis() {
        return this;
    }
}
exports.TestClass = TestClass;
function getThis() {
    return this;
}
exports.getThis = getThis;
function getThisAsFunction() {
    return this;
}
exports.getThisAsFunction = getThisAsFunction;
async function getThisAsAsyncFunction() {
    return this;
}
exports.getThisAsAsyncFunction = getThisAsAsyncFunction;
function testFuncReturnStr() {
    return TEST_STRING;
}
exports.testFuncReturnStr = testFuncReturnStr;
class ReturnThisTestClass {
    constructor() {
        this.getThis = getThis.bind(this);
        this.getThisAsFunction = getThisAsFunction.bind(this);
        this.getThisAsAsyncFunction = getThisAsAsyncFunction.bind(this);
    }
}
exports.ReturnThisTestClass = ReturnThisTestClass;
exports.returnThisTestClassInstance = new ReturnThisTestClass();
const testClass = () => {
    let testClass = new TestClass();
    let testVal = testClass.getThis();
};
const test = () => {
    console.log(getThis.call(TEST_NUMBER), 
    getThis.call(TEST_STRING), 
    getThis.call(TEST_NULL), 
    getThis.call(TEST_MAP), 
    getThis.call(TEST_OBJ), 
    getThis.call(TEST_ARRAY), 
    getThis.call(TEST_TUPLE), 
    getThis.call(new TestClass()), 
    getThisAsFunction.call(testFuncReturnStr()));
};
const testAsync = async () => {
    console.log(await getThisAsAsyncFunction.call(testFuncReturnStr()));
};
testClass();
test();
testAsync();
