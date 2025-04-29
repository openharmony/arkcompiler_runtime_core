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

import {util} from "@ohos.util";

interface Person {
    name: string,
    age: number
}

function main(): int {
    const suite = new ArkTestsuite("CommonUtil getHash API tests")

    suite.addTest("Get the hash value of an Interface object", testUtilGetHash001)
    suite.addTest("Get the hash value of an array of numbers", testUtilGetHash002)
    suite.addTest("Get the hash value of a Class object", testUtilGetHash003)
    suite.addTest("Get the hash value of a Date object", testUtilGetHash004)
    suite.addTest("Get the hash value of a RegExp object", testUtilGetHash005)
    suite.addTest("Get the hash value of a Math object", testUtilGetHash006)
    suite.addTest("Get the hash value of a Math object and a string", testUtilGetHash007)
    suite.addTest("Get the hash value of two Class object", testUtilGetHash008)
    suite.addTest("Get the hash value of a number data and a string", testUtilGetHash009)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_GET_HASH_001
 * @tc.name: testUtilGetHash001
 * @tc.desc: Get the hash value of an Interface object.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testUtilGetHash001() {
    let person: Person = {
        name: "Alice",
        age: 30
    };
    let result1 = util.getHash(person);
    let result2 = util.getHash(person);
    assertEQ(result1, result2, "The same object should have the same hash code");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_GET_HASH_002
 * @tc.name: testUtilGetHash002
 * @tc.desc: Get the hash value of an array of numbers.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testUtilGetHash002() {
    let numbers = [1, 2, 3, 4, 5];
    let result1 = util.getHash(numbers);
    let result2 = util.getHash(numbers);
    assertEQ(result1, result2, "The same object should have the same hash code");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_GET_HASH_003
 * @tc.name: testUtilGetHash003
 * @tc.desc: Get the hash value of a Class object.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testUtilGetHash003() {
    class Car {
        make: string;
        model: string;

        constructor(make: string, model: string) {
          this.make = make;
          this.model = model;
        }
    }
    let myCar = new Car("Toyota", "Corolla");
    let result1 = util.getHash(myCar);
    let result2 = util.getHash(myCar);
    assertEQ(result1, result2, "The same object should have the same hash code");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_GET_HASH_004
 * @tc.name: testUtilGetHash004
 * @tc.desc: Get the hash value of a Date object.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testUtilGetHash004() {
    let now = new Date();
    let result1 = util.getHash(now);
    let result2 = util.getHash(now);
    assertEQ(result1, result2, "The same object should have the same hash code");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_GET_HASH_005
 * @tc.name: testUtilGetHash005
 * @tc.desc: Get the hash value of a RegExp object.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testUtilGetHash005() {
    let regex = new RegExp("abc");
    let result1 = util.getHash(regex);
    let result2 = util.getHash(regex);
    assertEQ(result1, result2, "The same object should have the same hash code");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_GET_HASH_006
 * @tc.name: testUtilGetHash006
 * @tc.desc: Get the hash value of a Math object.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testUtilGetHash006() {
    let mathObject: Math = new Math();
    let result1 = util.getHash(mathObject);
    let result2 = util.getHash(mathObject);
    assertEQ(result1, result2, "The same object should have the same hash code");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_GET_HASH_007
 * @tc.name: testUtilGetHash007
 * @tc.desc: Get the hash value of a Math object and a string.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testUtilGetHash007() {
    let mathObject: Math = new Math();
    let test: string = "test";
    let result1 = util.getHash(mathObject);
    let result2 = util.getHash(test);
    assertTrue(result1 !== result2, "The different object should have the different hash code");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_GET_HASH_008
 * @tc.name: testUtilGetHash008
 * @tc.desc: Get the hash value of two Class object.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testUtilGetHash008() {
    let person: Person = {
        name: "Alice",
        age: 30
    };
    let person1: Person = {
        name: "Lily",
        age: 25
    };
    let result1 = util.getHash(person);
    let result2 = util.getHash(person1);
    assertTrue(result1 !== result2, "The different object should have the different hash code");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_GET_HASH_009
 * @tc.name: testUtilGetHash009
 * @tc.desc: Get the hash value of a number data and a string.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testUtilGetHash009() {
    let data: number = 11;
    let test: string = "test";
    let result1 = util.getHash(data);
    let result2 = util.getHash(test);
    assertTrue(result1 !== result2, "The different object should have the different hash code");
}
