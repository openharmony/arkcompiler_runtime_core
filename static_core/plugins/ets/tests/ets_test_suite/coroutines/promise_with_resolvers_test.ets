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

async function testPWRTest() {
    let pwr1 = new PromiseWithResolvers<int>();
    pwr1.resolve(1);

    arktest.assertEQ(await pwr1.promise, 1);

    let pwr2 = new PromiseWithResolvers<int>();
    pwr2.reject(new Error('Test error'));
    try {
        await pwr2.promise;
    } catch (e) {
        arktest.assertEQ(e.toString(), 'Error: Test error');
    }
}

async function main(): Promise<int> {
    let testSuite = new arktest.ArkTestsuite('coroutines.promise_with_resolvers_test');
    testSuite.addAsyncTest('testPWRTest', testPWRTest);
    let res = await testSuite.runAsync();
    return res;
}