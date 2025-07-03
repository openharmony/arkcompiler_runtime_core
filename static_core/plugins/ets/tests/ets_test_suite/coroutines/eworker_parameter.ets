/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
    public a:string = "";
}

function tesObject(a:TestA) : TestA {
    a.a = "bbb";
    return a;
}

function testReturnArray(a:TestA, b:TestA, c:string) : Array<TestA> {
    a.a = "bbb";
    b.a = c;
    let arr = new Array<TestA>();
    arr.push(a);
    arr.push(b);
    return arr;
}

function testArrayObjects(arr:Array<TestA>) : Array<TestA> {
    arr[0].a = "value1";
    arr[1].a = "value2";
    return arr;
}

function testPrimitive(a:int, b:string, c:boolean, d:double) {
    assertEQ(a, 1);
    assertEQ(b, "bbb");
    assertEQ(c, true);
    assertEQ(d, 1.0);
}

function singleObject():void {
    let job = eaw.run<TestA>(tesObject, new TestA());
    assertEQ(job.Await().a, "bbb");
}

function returnArray():void {
    let a1 = new TestA();
    let a2 = new TestA();
    let job = eaw.run<Array<TestA>>(testReturnArray, a1, a2, "ccc");
    let arr = job.Await();
    assertEQ(arr[0].a, "bbb");
    assertEQ(arr[1].a, "ccc");
    assertEQ(a1.a, "bbb");
    assertEQ(a2.a, "ccc");
}

function arrayParameter():void {
    let arr:Array<TestA> = new Array<TestA>();
    let a1 = new TestA();
    let a2 = new TestA();
    arr.push(a1);
    arr.push(a2);
    let job = eaw.run<Array<TestA>>(testArrayObjects, arr);
    let arr2 = job.Await();
    assertEQ(arr2[0].a, "value1");
    assertEQ(arr2[1].a, "value2");
    assertEQ(arr2, arr);
    assertEQ(a1.a, "value1");
    assertEQ(a2.a, "value2");
}

function primitiveParameter() : void {
    eaw.run<void>(testPrimitive, 1, "bbb", true, 1.0).Await();
}


let eaw:EAWorker = new EAWorker();
function main() {
    const testSuite = new ArkTestsuite("EAWorkerParameterTest");
    testSuite.addTest("singleObject", singleObject);
    testSuite.addTest("returnArray", returnArray);
    testSuite.addTest("arrayParameter", arrayParameter);
    testSuite.addTest("primitiveParameter", primitiveParameter);
    testSuite.run();
    eaw.join();
}
