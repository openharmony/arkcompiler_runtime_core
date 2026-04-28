/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class TestA {
    public a:string = '';
}

function tesObject(a:TestA) : TestA {
    a.a = 'bbb';
    return a;
}

function testReturnArray(a:TestA, b:TestA, c:string) : Array<TestA> {
    a.a = 'bbb';
    b.a = c;
    let arr = new Array<TestA>();
    arr.push(a);
    arr.push(b);
    return arr;
}

function testArrayObjects(arr:Array<TestA>) : Array<TestA> {
    arr[0].a = 'value1';
    arr[1].a = 'value2';
    return arr;
}

function testPrimitive(a:int, b:string, c:boolean, d:double) {
    arktest.assertEQ(a, 1);
    arktest.assertEQ(b, 'bbb');
    arktest.assertEQ(c, true);
    arktest.assertEQ(d, 1.0);
}

async function singleObject(): Promise<void> {
    let p = eaw.run<TestA>(tesObject, new TestA());
    arktest.assertEQ((await p).a, 'bbb');
}

async function returnArray(): Promise<void> {
    let a1 = new TestA();
    let a2 = new TestA();
    let p = eaw.run<Array<TestA>>(testReturnArray, a1, a2, 'ccc');
    let arr = await p;
    arktest.assertEQ(arr[0].a, 'bbb');
    arktest.assertEQ(arr[1].a, 'ccc');
    arktest.assertEQ(a1.a, 'bbb');
    arktest.assertEQ(a2.a, 'ccc');
}

async function arrayParameter(): Promise<void> {
    let arr:Array<TestA> = new Array<TestA>();
    let a1 = new TestA();
    let a2 = new TestA();
    arr.push(a1);
    arr.push(a2);
    let p = eaw.run<Array<TestA>>(testArrayObjects, arr);
    let arr2 = await p;
    arktest.assertEQ(arr2[0].a, 'value1');
    arktest.assertEQ(arr2[1].a, 'value2');
    arktest.assertEQ(arr2, arr);
    arktest.assertEQ(a1.a, 'value1');
    arktest.assertEQ(a2.a, 'value2');
}

async function primitiveParameter(): Promise<void> {
    await eaw.run<void>(testPrimitive, 1, 'bbb', true, 1.0);
}

async function undefinedTest(): Promise<void> {
    let callback = (arg?: int): boolean => {
        if (arg == undefined) {
            return true;
        }
        return false;
    }
    let p = eaw.run<boolean>(callback, undefined);
    let res = await p;
    arktest.assertEQ(res, true);
}

let eaw:EAWorker = new EAWorker();

async function main(): Promise<int> {
    eaw.start();
    const testSuite = new arktest.ArkTestsuite('EAWorkerParameterTest');
    testSuite.addAsyncTest('singleObject', singleObject);
    testSuite.addAsyncTest('returnArray', returnArray);
    testSuite.addAsyncTest('arrayParameter', arrayParameter);
    testSuite.addAsyncTest('primitiveParameter', primitiveParameter);
    testSuite.addAsyncTest("undefinedTest", undefinedTest);
    let res = testSuite.run();
    await eaw.join();
    return res;
}
