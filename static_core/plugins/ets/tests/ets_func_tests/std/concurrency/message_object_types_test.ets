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


function testMessageWithBasicTypes() {
    let eaw = new EAWorker();
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {}, eaw);

    let msgInt = new concurrency.Message(1, 42 as int, handler);
    let msgNumber = new concurrency.Message(2, 3.14, handler);
    let msgBool = new concurrency.Message(3, true, handler);

    arktest.assertEQ(msgInt.getObject() as int, 42);
    arktest.assertEQ(msgNumber.getObject() as number, 3.14);
    arktest.assertEQ(msgBool.getObject() as boolean, true);
}

function testMessageWithString() {
    let eaw = new EAWorker();
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {}, eaw);

    let msg = new concurrency.Message(10, "hello world", handler);
    arktest.assertEQ(msg.getObject() as string, "hello world");
}

function testMessageWithDate() {
    let eaw = new EAWorker();
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {}, eaw);

    let date = new Date(2025, 0, 1);
    let msg = new concurrency.Message(20, date, handler);
    let received = msg.getObject() as Date;
    arktest.assertEQ(received.getFullYear(), 2025);
}

function testMessageWithRegExp() {
    let eaw = new EAWorker();
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {}, eaw);

    let reg = new RegExp("abc", "g");
    let msg = new concurrency.Message(30, reg, handler);
    let received = msg.getObject() as RegExp;
    arktest.assertTrue(received.test("abc123"));
}

function testMessageWithArray() {
    let eaw = new EAWorker();
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {}, eaw);

    let arr: int[] = [1, 2, 3];
    let msg = new concurrency.Message(40, arr, handler);
    arktest.assertEQ(msg.getObject(), arr);
}

class TestObj {
    name: string = ""
    value: int = 0
    constructor(name: string, value: int) {
        this.name = name
        this.value = value
    }
}

function testMessageWithObject() {
    let eaw = new EAWorker();
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {}, eaw);

    let obj = new TestObj("test", 100);
    let msg = new concurrency.Message(50, obj, handler);
    let received = msg.getObject() as TestObj;
    arktest.assertEQ(received.name, "test");
    arktest.assertEQ(received.value, 100);
}

function testMessageWithMap() {
    let eaw = new EAWorker();
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {}, eaw);

    let map = new Map<string, int>();
    map.set("a", 1);
    map.set("b", 2);
    let msg = new concurrency.Message(60, map, handler);
    arktest.assertEQ(msg.getObject(), map);
}

function testMessageWithUndefined() {
    let eaw = new EAWorker();
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {}, eaw);

    let msg = new concurrency.Message(70, undefined, handler);
    arktest.assertEQ(msg.getObject(), undefined);
}

function testMessageWitNull() {
    let eaw = new EAWorker();
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {}, eaw);

    let msg = new concurrency.Message(80, null, handler);
    arktest.assertEQ(msg.getObject(), null);
}

function main() {
    let testSuite = new arktest.ArkTestsuite('MessageObjectTypes');
    testSuite.addTest('testMessageWithBasicTypes', testMessageWithBasicTypes);
    testSuite.addTest('testMessageWithString', testMessageWithString);
    testSuite.addTest('testMessageWithDate', testMessageWithDate);
    testSuite.addTest('testMessageWithRegExp', testMessageWithRegExp);
    testSuite.addTest('testMessageWithArray', testMessageWithArray);
    testSuite.addTest('testMessageWithObject', testMessageWithObject);
    testSuite.addTest('testMessageWithMap', testMessageWithMap);
    testSuite.addTest('testMessageWithUndefined', testMessageWithUndefined);
    testSuite.addTest('testMessageWitNull', testMessageWitNull);
    return testSuite.run();
}
