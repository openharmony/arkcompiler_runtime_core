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

import {util} from "@ohos.util"

let type = new util.types();
class forTest {
    static async asyncStaticMethod() {}
    static notAsyncStaticMethod(): Object { return {} }
    static promiseMethod(): Promise<Function> { return new Promise<Function>(() => {}) }

    async asyncMethod() {}
    notAsyncMethod(): Object { return {} }
    promiseInstanceMethod(): Promise<int> { return Promise.resolve(1) }
}

async function asyncFunc() {}
function notAsyncFunc(): Object { return {} }

function testAsyncArg(f: () => Object): boolean {
    return type.isAsyncFunction(f)
}

async function asyncWithParams(): Promise<Function> { 
    return new Promise<Function>(() => {}) 
}

async function asyncReturningPromise(): Promise<int> { 
    return Promise.resolve(42) 
}

function testAsyncWithTrue() {
    let x = async () => {}
    arktest.assertTrue(type.isAsyncFunction(x), 'async arrow function should be detected as async')
    arktest.assertTrue(testAsyncArg(x), 'async arrow function should pass testAsyncArg')

    arktest.assertTrue(type.isAsyncFunction(asyncFunc), 'async function should be detected as async')
    arktest.assertTrue(testAsyncArg(asyncFunc), 'async function should pass testAsyncArg')

    arktest.assertTrue(type.isAsyncFunction(forTest.asyncStaticMethod), 'async static method should be detected as async')
    arktest.assertTrue(testAsyncArg(forTest.asyncStaticMethod), 'async static method should pass testAsyncArg')

    let ft = new forTest()
    arktest.assertTrue(type.isAsyncFunction(ft.asyncMethod), 'async instance method should be detected as async')
    arktest.assertTrue(testAsyncArg(ft.asyncMethod), 'async instance method should pass testAsyncArg')

    arktest.assertTrue(type.isAsyncFunction(asyncWithParams), 'async function with parameters should be detected as async')
    arktest.assertTrue(testAsyncArg(asyncWithParams), 'async function with parameters should pass testAsyncArg')

    arktest.assertTrue(type.isAsyncFunction(asyncReturningPromise), 'async function returning Promise should be detected as async')
    arktest.assertTrue(testAsyncArg(asyncReturningPromise), 'async function returning Promise should pass testAsyncArg')
}

function testAsyncWithFalse() {
    let y = (): Object => { return {} }
    arktest.assertFalse(type.isAsyncFunction(y), 'non-async arrow function should not be detected as async')
    arktest.assertFalse(testAsyncArg(y), 'non-async arrow function should fail testAsyncArg')

    arktest.assertFalse(type.isAsyncFunction(notAsyncFunc), 'non-async function should not be detected as async')
    arktest.assertFalse(testAsyncArg(notAsyncFunc), 'non-async function should fail testAsyncArg')

    arktest.assertFalse(type.isAsyncFunction(forTest.notAsyncStaticMethod), 'non-async static method should not be detected as async')
    arktest.assertFalse(testAsyncArg(forTest.notAsyncStaticMethod), 'non-async static method should fail testAsyncArg')

    let ft = new forTest()
    arktest.assertFalse(type.isAsyncFunction(ft.notAsyncMethod), 'non-async instance method should not be detected as async')
    arktest.assertFalse(testAsyncArg(ft.notAsyncMethod), 'non-async instance method should fail testAsyncArg')

    arktest.assertFalse(type.isAsyncFunction(ft.promiseInstanceMethod), 'instance method returning Promise should not be detected as async')
    arktest.assertFalse(testAsyncArg(ft.promiseInstanceMethod), 'instance method returning Promise should fail testAsyncArg')

    let promiseFunc = (): Promise<Function> => { return new Promise<Function>(() => {}) }
    arktest.assertFalse(type.isAsyncFunction(promiseFunc), 'function returning Promise should not be detected as async')
    arktest.assertFalse(testAsyncArg(promiseFunc), 'function returning Promise should fail testAsyncArg')

    let promiseArrow = (): Promise<int> => { return Promise.resolve(1) }
    arktest.assertFalse(type.isAsyncFunction(promiseArrow), 'arrow function returning Promise should not be detected as async')
    arktest.assertFalse(testAsyncArg(promiseArrow), 'arrow function returning Promise should fail testAsyncArg')

    arktest.assertFalse(type.isAsyncFunction(forTest.promiseMethod), 'method returning Promise should not be detected as async')
    arktest.assertFalse(testAsyncArg(forTest.promiseMethod), 'method returning Promise should fail testAsyncArg')
}
function main(): int {
    const suite = new arktest.ArkTestsuite("util type.isAsyncFunction API test")

    suite.addTest("Test type.isAsyncFunction API With True", testAsyncWithTrue)
    suite.addTest("Test type.isAsyncFunction API With False", testAsyncWithFalse)

    return suite.run()
}