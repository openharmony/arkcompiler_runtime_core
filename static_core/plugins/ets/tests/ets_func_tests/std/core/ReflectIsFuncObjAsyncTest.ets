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

class forTest {
    static async asyncStaticMethod() {}
    static notAsyncStaticMethod(): Object { return {} }

    async asyncMethod() {}
    notAsyncMethod(): Object { return {} }
}

async function asyncFunc() {}
function notAsyncFunc(): Object { return {} }

function testAsyncArg(f: () => Object): boolean {
    return reflect.isFuncObjAsync(f)
}

function testAsync() {
    let x = async () => {}
    arktest.assertTrue(reflect.isFuncObjAsync(x))
    arktest.assertTrue(testAsyncArg(x))
    let y = (): Object => { return {} }
    arktest.assertFalse(reflect.isFuncObjAsync(y))
    arktest.assertFalse(testAsyncArg(y))

    arktest.assertTrue(reflect.isFuncObjAsync(asyncFunc))
    arktest.assertTrue(testAsyncArg(asyncFunc))
    arktest.assertFalse(reflect.isFuncObjAsync(notAsyncFunc))
    arktest.assertFalse(testAsyncArg(notAsyncFunc))

    arktest.assertTrue(reflect.isFuncObjAsync(forTest.asyncStaticMethod))
    arktest.assertTrue(testAsyncArg(forTest.asyncStaticMethod))
    arktest.assertFalse(reflect.isFuncObjAsync(forTest.notAsyncStaticMethod))
    arktest.assertFalse(testAsyncArg(forTest.notAsyncStaticMethod))

    let ft = new forTest()
    arktest.assertTrue(reflect.isFuncObjAsync(ft.asyncMethod))
    arktest.assertTrue(testAsyncArg(ft.asyncMethod))
    arktest.assertFalse(reflect.isFuncObjAsync(ft.notAsyncMethod))
    arktest.assertFalse(testAsyncArg(ft.notAsyncMethod))
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Reflect isFuncObjAsync API test")

    suite.addTest("Test isFuncObjAsync API", testAsync)

    return suite.run()
}
