/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

import {ArrayList} from "@ohos.util.ArrayList";

class Temp {
    age: number = 0;
    name: string = "";
}

function main() {
    const suite = new ArkTestsuite("ArrayList API tests");

    suite.addTest("Testing ArrayList length is ok", testArrayListLength0001);
    suite.addTest("Testing ArrayList constructor support dataType", testArrayListConstructor0001);
    suite.addTest("Testing ArrayList add special string", testArrayListAdd0002);
    suite.addTest("Testing ArrayList insert data in start and end", testArrayListInsert0001);
    suite.addTest("Testing ArrayList insert special string", testArrayListInsert0002);
    suite.addTest("Testing ArrayList check has data", testArrayListHas0001);
    suite.addTest("Testing ArrayList getIndexOf", testArrayListGetIndexOf0001);
    suite.addTest("Testing ArrayList getLastIndexOf", testArrayListGetLastIndexOf0001);

    return suite.run();
}

function testArrayListLength0001() {
    const arrayList = new ArrayList<number>();
    assertEQ(arrayList.length, 0, "The length of a newly created ArrayList should be 0");
}

function testArrayListConstructor0001() {
    let arrayList = new ArrayList<number | string | Array<number | string | boolean> | boolean | bigint | Int8Array |
    Int16Array | Int32Array | undefined | null | Uint8Array | Uint16Array | Uint32Array |
    BigInt64Array | BigUint64Array>();
    assertEQ(arrayList.length, 0, "The length of a newly created ArrayList with various data types should be 0");
}

function testArrayListAdd0002() {
    let arrayList = new ArrayList<number | string | Array<number | string | boolean> | boolean | undefined | null |
    Object>();
    arrayList.add("");
    arrayList.add("`");
    arrayList.add("%");
    arrayList.add(`"`);

    assertEQ(arrayList[0], "", "The first element should be an empty string");
    assertEQ(arrayList[1], "`", "The second element should be '`'");
    assertEQ(arrayList[2], "%", "The third element should be '%'");
    assertEQ(arrayList[3], `"`, "The fourth element should be '\"'");
}

function testArrayListInsert0001() {
    let arrayList = new ArrayList<number | string>();
    arrayList.add("zhang");
    arrayList.insert("start", 0);
    arrayList.insert("end", arrayList.length);
    let startIndex = arrayList.getIndexOf("start");
    arrayList.removeByIndex(startIndex);
    let endIndex = arrayList.getIndexOf("end");
    arrayList.removeByIndex(endIndex);
    assertFalse(arrayList.has("start"), "The ArrayList should not contain 'start' after removal");
    assertFalse(arrayList.has("end"), "The ArrayList should not contain 'end' after removal");
}

function testArrayListInsert0002() {
    let arrayList = new ArrayList<number | string>();
    arrayList.insert("`", 0);
    arrayList.insert("%", 1);
    arrayList.insert(`"`, 2);
    let index1 = arrayList.getIndexOf("`");
    arrayList.removeByIndex(index1);
    let index2 = arrayList.getIndexOf("%");
    arrayList.removeByIndex(index2);
    let index3 = arrayList.getIndexOf(`"`);
    arrayList.removeByIndex(index3);
    assertFalse(arrayList.has("`"), "The ArrayList should not contain '`' after removal");
    assertFalse(arrayList.has("%"), "The ArrayList should not contain '%' after removal");
    assertFalse(arrayList.has(`"`), "The ArrayList should not contain '\"' after removal");
}

function testArrayListHas0001() {
    let arrayList = new ArrayList<number | string | Array<number | string | boolean> | boolean | undefined | null |
    Object>();
    arrayList.add("zhang san");
    arrayList.add(1);
    let temp: Temp = {
        age: 10, name: "zhang"
    };
    arrayList.add(temp);
    arrayList.add(undefined);
    arrayList.add(null);
    arrayList.add(true);
    let array = [0, 1, 2, undefined];
    arrayList.add(array);
    arrayList.add("");
    arrayList.add("`");
    arrayList.add("%");
    arrayList.add(`"`);
    assertTrue(arrayList.has("zhang san"), "The ArrayList should contain 'zhang san'");
    assertTrue(arrayList.has(1), "The ArrayList should contain the number 1");
    assertTrue(arrayList.has(temp), "The ArrayList should contain the Temp object");
    assertTrue(arrayList.has(undefined), "The ArrayList should contain undefined");
    assertTrue(arrayList.has(null), "The ArrayList should contain null");
    assertTrue(arrayList.has(true), "The ArrayList should contain true");
    assertTrue(arrayList.has(array), "The ArrayList should contain the array [0, 1, 2, undefined]");
    assertTrue(arrayList.has(""), "The ArrayList should contain an empty string");
    assertTrue(arrayList.has("`"), "The ArrayList should contain '`'");
    assertTrue(arrayList.has("%"), "The ArrayList should contain '%'");
    assertTrue(arrayList.has(`"`), "The ArrayList should contain '\"'");
    arrayList.remove("zhang san");
    arrayList.remove(1);
    arrayList.remove(temp);
    arrayList.remove(undefined);
    arrayList.remove(null);
    arrayList.remove(true);
    arrayList.remove(array);
    arrayList.remove("");
    arrayList.remove("`");
    arrayList.remove("%");
    arrayList.remove(`"`);
    assertFalse(arrayList.has("zhang san"), "The ArrayList should not contain 'zhang san' after removal");
    assertFalse(arrayList.has(1), "The ArrayList should not contain the number 1 after removal");
    assertFalse(arrayList.has(temp), "The ArrayList should not contain the Temp object after removal");
    assertFalse(arrayList.has(undefined), "The ArrayList should not contain undefined after removal");
    assertFalse(arrayList.has(null), "The ArrayList should not contain null after removal");
    assertFalse(arrayList.has(true), "The ArrayList should not contain true after removal");
    assertFalse(arrayList.has(array), "The ArrayList should not contain the array [0, 1, 2, undefined] after removal");
    assertFalse(arrayList.has(""), "The ArrayList should not contain an empty string after removal");
    assertFalse(arrayList.has("`"), "The ArrayList should not contain '`' after removal");
    assertFalse(arrayList.has("%"), "The ArrayList should not contain '%' after removal");
    assertFalse(arrayList.has(`"`), "The ArrayList should not contain '\"' after removal");
}

function testArrayListGetIndexOf0001() {
    let arrayList = new ArrayList<number | string | Array<number | string | boolean> | boolean | undefined | null |
    Object>();
    arrayList.add("zhang san");
    arrayList.add(1);
    let temp: Temp = {
        age: 10, name: "zhang"
    };
    arrayList.add(temp);
    arrayList.add(undefined);
    arrayList.add(null);
    arrayList.add(true);
    let array = [0, 1, 2, undefined];
    arrayList.add(array);
    arrayList.add("");
    arrayList.add("`");
    arrayList.add("%");
    arrayList.add(`"`);
    arrayList.add("zhang san");
    arrayList.add(1);
    let index1 = arrayList.getIndexOf("zhang san");
    assertEQ(index1, 0, "The index of 'zhang san' should be 0");
    let index2 = arrayList.getIndexOf(1);
    assertEQ(index2, 1, "The index of the number 1 should be 1");
    arrayList.remove(1);
    arrayList.remove("zhang san");
    assertTrue(arrayList.has("zhang san"),
        "The ArrayList should still contain 'zhang san' after removing another instance");
    assertTrue(arrayList.has(1), "The ArrayList should still contain the number 1 after removing another instance");
}

function testArrayListGetLastIndexOf0001() {
    let arrayList = new ArrayList<number | string | Array<number | string | boolean> | boolean | undefined | null |
    Object>();
    arrayList.add("zhang san");
    arrayList.add(1);
    let temp: Temp = {
        age: 10, name: "zhang"
    };
    arrayList.add(temp);
    arrayList.add(undefined);
    arrayList.add(null);
    arrayList.add(true);
    let array = [0, 1, 2, undefined];
    arrayList.add(array);
    arrayList.add("");
    arrayList.add("`");
    arrayList.add("%");
    arrayList.add(`"`);
    arrayList.add("zhang san");
    arrayList.add(1);
    let index1 = arrayList.getLastIndexOf("zhang san");
    assertEQ(index1, 11, "The last index of 'zhang san' should be 11");
    let index2 = arrayList.getLastIndexOf(1);
    assertEQ(index2, 12, "The last index of the number 1 should be 12");
    arrayList.remove(1);
    arrayList.remove("zhang san");
    assertTrue(arrayList.has("zhang san"),
        "The ArrayList should still contain 'zhang san' after removing another instance");
    assertTrue(arrayList.has(1), "The ArrayList should still contain the number 1 after removing another instance");
}
