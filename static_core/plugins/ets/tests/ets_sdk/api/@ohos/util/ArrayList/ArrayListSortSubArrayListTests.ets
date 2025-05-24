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
import {BusinessError} from "@ohos.base";

const OutOfBoundsErrorCodeId: number = 10200001;

function main() {
    const suite = new ArkTestsuite("ArrayList API tests");

    suite.addTest("ArrayList sort", testSortNumbers);
    suite.addTest("ArrayList sort with different types in ascending order", testSortDifferentTypesAscending);
    suite.addTest("ArrayList sort with change the container", testSortWithContainerChange);
    suite.addTest("ArrayList sort with different types in descending order", testSortDifferentTypesDescending);
    suite.addTest("ArrayList sort with stores multiple types of data", testSortMultipleTypes);
    suite.addTest("ArrayList subArrayList", testSubArrayList);
    suite.addTest("ArrayList subArrayList with start index out of range", testSubArrayListStartIndexOutOfRange);
    suite.addTest("ArrayList subArrayList with end index out of range", testSubArrayListEndIndexOutOfRange);
    suite.addTest("ArrayList subArrayList with end index smaller than start index",
        testSubArrayListEndIndexSmallerThanStartIndex);
    suite.addTest("ArrayList subArrayList with change subArrayList", testSubArrayListWithChange);
    suite.addTest("ArrayList subArrayList with empty container", testSubArrayListEmptyContainer);
    suite.addTest("ArrayList subArrayList with start index = 0 and end index = length - 1", testSubArrayListFullRange);

    return suite.run();
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber3: number = 3;
    static readonly testNumber5: number = 5;
    static readonly testNumber8: number = 8;
    static readonly forNumber: Number[] = [1, 2, 3, 4, 5];
    static readonly forString: String[] = ["ssss", "2", "%", "Z"];
    static readonly forBoolean: Boolean[] = [true, false, false, true];
    static readonly forObject: Object[] = [[1], [2, 3, 4], [1, 2], [5]];
    static readonly result1: Number[] = [5, 4, 3, 2, 1];
    static readonly result6: Number[] = [2, 3, 4];
    static readonly result7: Number[] = [2, 3, 4, 5];
}

function testSortNumbers() {
    let arrayList1 = new ArrayList<number>();
    initializeArrayList(arrayList1, TestData.result1);
    let array1 = arrayList1.convertToArray();
    arrayList1.sort();
    array1.sort();
    for (let i: number = 0; i < arrayList1.length; i++) {
        assertEQ(arrayList1[i], array1[i], `Element at index ${i} should match`);
    }

    let arrayList2 = new ArrayList<string>();
    initializeArrayList(arrayList2, TestData.forString);
    let array2 = arrayList2.convertToArray();
    arrayList2.sort();
    array2.sort();
    for (let i: number = 0; i < arrayList2.length; i++) {
        assertEQ(arrayList2[i], array2[i], `Element at index ${i} should match`);
    }

    let arrayList3 = new ArrayList<boolean>();
    initializeArrayList(arrayList3, TestData.forBoolean);
    let array3 = arrayList3.convertToArray();
    arrayList3.sort();
    array3.sort();
    for (let i: number = 0; i < arrayList3.length; i++) {
        assertEQ(arrayList3[i], array3[i], `Element at index ${i} should match`);
    }

    let arrayList4 = new ArrayList<object>();
    initializeArrayList(arrayList4, TestData.forObject);
    let array4 = arrayList4.convertToArray();
    arrayList4.sort();
    array4.sort();
    for (let i: number = 0; i < arrayList4.length; i++) {
        assertEQ(arrayList4[i], array4[i], `Element at index ${i} should match`);
    }
}

function testSortMultipleTypes() {
    let arrayList = new ArrayList<number | string | boolean | object>();
    for (let i: int = 0; i < TestData.forNumber.length; i++) {
        arrayList.add(TestData.forNumber[i]);
    }
    for (let i: int = 0; i < TestData.forString.length; i++) {
        arrayList.add(TestData.forString[i]);
    }
    for (let i: int = 0; i < TestData.forBoolean.length; i++) {
        arrayList.add(TestData.forBoolean[i]);
    }
    for (let i: int = 0; i < TestData.forObject.length; i++) {
        arrayList.add(TestData.forObject[i]);
    }
    let array = arrayList.convertToArray();
    arrayList.sort();
    array.sort();
    for (let i: number = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], array[i], `Element at index ${i} should match`);
    }
}

function testSortDifferentTypesAscending() {
    let arrayList1 = new ArrayList<number>();
    initializeArrayList(arrayList1, TestData.result1);
    arrayList1.sort((a: number, b: number) => a - b);
    for (let i: int = 0; i < arrayList1.length; i++) {
        assertEQ(arrayList1[i], TestData.forNumber[i], `Element at index ${i} should match`);
    }

    let arrayList2 = new ArrayList<string>();
    initializeArrayList(arrayList2, TestData.forString);
    let array2 = arrayList2.convertToArray();
    arrayList2.sort((a: string, b: string) => a.compareTo(b));
    array2.sort((a: string, b: string) => a.compareTo(b));
    for (let i: number = 0; i < arrayList2.length; i++) {
        assertEQ(arrayList2[i], array2[i], `Element at index ${i} should match`);
    }

    let arrayList3 = new ArrayList<boolean>();
    initializeArrayList(arrayList3, TestData.forBoolean);
    let array3 = arrayList3.convertToArray();
    arrayList3.sort((a: boolean, b: boolean) => (a ? 1 : 0) - (b ? 1 : 0));
    array3.sort((a: boolean, b: boolean) => (a ? 1 : 0) - (b ? 1 : 0));
    for (let i: number = 0; i < arrayList3.length; i++) {
        assertEQ(arrayList3[i], array3[i], `Element at index ${i} should match`);
    }

    let arrayList4 = new ArrayList<object>();
    initializeArrayList(arrayList4, TestData.forObject);
    let array4 = arrayList4.convertToArray();
    arrayList4.sort((a: object, b: object): number => {
        let sa = new String(a);
        let sb = new String(b);
        return sa.compareTo(sb);
    });
    array4.sort((a: object, b: object): number => {
        let sa = new String(a);
        let sb = new String(b);
        return sa.compareTo(sb);
    });
    for (let i: number = 0; i < arrayList4.length; i++) {
        assertEQ(arrayList4[i], array4[i], `Element at index ${i} should match`);
    }
}

function testSortWithContainerChange() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.result1);
    arrayList.sort((a: number, b: number) => {
        arrayList.clear();
        return a - b;
    });
    assertEQ(arrayList.length, 0, "ArrayList length should be 0 after clear");
}

function testSortDifferentTypesDescending() {
    let arrayList1 = new ArrayList<number>();
    initializeArrayList(arrayList1, TestData.forNumber);
    arrayList1.sort((a: number, b: number) => b - a);
    for (let i: int = 0; i < arrayList1.length; i++) {
        assertEQ(arrayList1[i], TestData.result1[i], `Element at index ${i} should match`);
    }

    let arrayList2 = new ArrayList<string>();
    initializeArrayList(arrayList2, TestData.forString);
    let array2 = arrayList2.convertToArray();
    arrayList2.sort((a: string, b: string) => b.compareTo(a));
    array2.sort((a: string, b: string) => b.compareTo(a));
    for (let i: number = 0; i < arrayList2.length; i++) {
        assertEQ(arrayList2[i], array2[i], `Element at index ${i} should match`);
    }

    let arrayList3 = new ArrayList<boolean>();
    initializeArrayList(arrayList3, TestData.forBoolean);
    let array3 = arrayList3.convertToArray();
    arrayList3.sort((a: boolean, b: boolean) => (b ? 1 : 0) - (a ? 1 : 0));
    array3.sort((a: boolean, b: boolean) => (b ? 1 : 0) - (a ? 1 : 0));
    for (let i: number = 0; i < arrayList3.length; i++) {
        assertEQ(arrayList3[i], array3[i], `Element at index ${i} should match`);
    }

    let arrayList4 = new ArrayList<object>();
    initializeArrayList(arrayList4, TestData.forObject);
    let array4 = arrayList4.convertToArray();
    arrayList4.sort((a: object, b: object): number => {
        let sa = new String(a);
        let sb = new String(b);
        return sb.compareTo(sa);
    });
    array4.sort((a: object, b: object): number => {
        let sa = new String(a);
        let sb = new String(b);
        return sb.compareTo(sa);
    });
    for (let i: number = 0; i < arrayList4.length; i++) {
        assertEQ(arrayList4[i], array4[i], `Element at index ${i} should match`);
    }
}

function testSubArrayList() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let subArrayList = arrayList.subArrayList(TestData.testNumber1, TestData.testNumber5 - TestData.testNumber1);
    for (let i: int = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.forNumber[i], `Element at index ${i} should match`);
    }
    for (let i: int = 0; i < subArrayList.length; i++) {
        assertEQ(subArrayList[i], TestData.result6[i], `Element at index ${i} should match`);
    }
}

function testSubArrayListStartIndexOutOfRange() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            "The value of \"fromIndex\" is out of range. It must be >= 0 && <= 4. Received value is: -1") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    expectThrow(() => {arrayList.subArrayList(-TestData.testNumber1, TestData.testNumber5)}, exceptionCheck);
}

function testSubArrayListEndIndexOutOfRange() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            "The value of \"toIndex\" is out of range. It must be >= 0 && <= 5. Received value is: 8") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    expectThrow(() => {arrayList.subArrayList(TestData.testNumber1, TestData.testNumber8)}, exceptionCheck);
}

function testSubArrayListEndIndexSmallerThanStartIndex() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            "The value of \"fromIndex\" is out of range. It must be >= 0 && <= 0. Received value is: 5") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    expectThrow(() => {arrayList.subArrayList(TestData.testNumber5, TestData.testNumber1)}, exceptionCheck);
}

function testSubArrayListWithChange() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let subArrayList = arrayList.subArrayList(TestData.testNumber1, TestData.testNumber5 - TestData.testNumber1);
    subArrayList.add(TestData.testNumber5);
    for (let i: int = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.forNumber[i], `Element at index ${i} should match`);
    }
    for (let i: int = 0; i < subArrayList.length; i++) {
        assertEQ(subArrayList[i], TestData.result7[i], `Element at index ${i} should match`);
    }
}

function testSubArrayListEmptyContainer() {
    let arrayList = new ArrayList<number>();
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message == "Container is empty") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    expectThrow(() => {arrayList.subArrayList(TestData.testNumber1, TestData.testNumber3)}, exceptionCheck);
}

function testSubArrayListFullRange() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let subArrayList = arrayList.subArrayList(0, TestData.testNumber5);
    for (let i: number = 0; i < subArrayList.length; i++) {
        assertEQ(subArrayList[i], arrayList[i], `Element at index ${i} should match`);
    }
    assertEQ(subArrayList.length, TestData.testNumber5, "SubArrayList length should be 5");
}

function initializeArrayList<T>(arrayList: ArrayList<T>, sourceArray: T[]) {
    for (let i = 0; i < sourceArray.length; i++) {
        arrayList.add(sourceArray[i]);
    }
}
