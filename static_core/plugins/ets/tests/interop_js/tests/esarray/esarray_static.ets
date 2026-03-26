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

import { es, ESUser, returnESUserArray, testUserArrayName, testUserArrayAge,
        returnFlatArray, esFuncArrayTest, returnSameArray, createEmptyArray } from 'testDecl';

function testGetLength(): boolean {
    let esArr1: es.Array<string> = createEmptyArray<string>();
    arktest.assertEQ(esArr1.length, 0);
    return true;
}

function testGetSetByIndex(): boolean {
    let esArr1: es.Array<string> = createEmptyArray<string>();
    arktest.assertEQ(typeof esArr1[0], 'undefined');
    esArr1[0] = 'test';
    arktest.assertEQ(esArr1.length, 1);
    arktest.assertEQ(esArr1[0], 'test');
    return true;
}

function testPopPush(): boolean {
    let esArr1: es.Array<string> = createEmptyArray<string>();
    esArr1.push('test1');
    esArr1.push('test2');
    arktest.assertEQ(esArr1.length, 2);
    arktest.assertEQ(esArr1[0], 'test1');
    arktest.assertEQ(esArr1[1], 'test2');
    arktest.assertEQ(esArr1.pop(), 'test2');
    arktest.assertEQ(esArr1.pop(), 'test1');
    arktest.assertTrue(esArr1.pop() === undefined);
    arktest.assertEQ(esArr1.length, 0);
    return true;
}

function testShiftUnshift(): boolean {
    let esArr1: es.Array<string> = createEmptyArray<string>();
    esArr1.unshift('test1');
    esArr1.unshift('test2');
    arktest.assertEQ(esArr1.length, 2);
    arktest.assertEQ(esArr1[0], 'test2');
    arktest.assertEQ(esArr1[1], 'test1');
    arktest.assertEQ(esArr1.shift(), 'test2');
    arktest.assertEQ(esArr1.shift(), 'test1');
    arktest.assertTrue(esArr1.shift() === undefined);
    arktest.assertEQ(esArr1.length, 0);
    return true;
}

function getOrderArray(length: int): es.Array<number> {
    let esArr1: es.Array<number> = createEmptyArray<number>();
    for (let i = 0; i < length; i++) {
        esArr1.push(i + 1);
    }
    return esArr1;
}

function testToString(): boolean {
    let esArr1 = getOrderArray(5);
    arktest.assertEQ(esArr1.toString(), '1,2,3,4,5');
    arktest.assertEQ(esArr1.toLocaleString(), '1,2,3,4,5');
    return true;
}

function testSort(): boolean {
    let esArr1: es.Array<number> = createEmptyArray<number>();
    esArr1.push(2);
    esArr1.push(4);
    esArr1.push(5);
    esArr1.push(1);
    esArr1.push(3);
    arktest.assertEQ(esArr1.toString(), '2,4,5,1,3');
    let esArr2 = esArr1.sort();
    arktest.assertEQ(esArr1.toString(), '1,2,3,4,5');
    let esArr3 = esArr1.sort((a: number, b: number) => {return b - a;});
    arktest.assertEQ(esArr3.toString(), '5,4,3,2,1');
    return true;
}

function testToSorted(): boolean {
    let esArr1: es.Array<number> = createEmptyArray<number>();
    esArr1.push(2);
    esArr1.push(4);
    esArr1.push(5);
    esArr1.push(1);
    esArr1.push(3);
    arktest.assertEQ(esArr1.toString(), '2,4,5,1,3');
    let esArr2 = esArr1.toSorted();
    arktest.assertEQ(esArr2.toString(), '1,2,3,4,5');
    let esArr3 = esArr1.toSorted((a: number, b: number) => {return b - a;});
    arktest.assertEQ(esArr3.toString(), '5,4,3,2,1');
    return true;
}

function testSplice(): boolean {
    let esArr1 = createEmptyArray<string>();
    esArr1.push('1');
    esArr1.push('2');
    esArr1.push('3');
    let esArr2 = esArr1.splice(1);
    arktest.assertEQ(esArr1.toString(), '1');
    arktest.assertEQ(esArr2.toString(), '2,3');
    let esArr3 = createEmptyArray<string>();
    esArr3.push('1');
    esArr3.push('2');
    esArr3.push('3');
    let esArr4 = esArr3.splice(1, 1);
    arktest.assertEQ(esArr3.toString(), '1,3');
    arktest.assertEQ(esArr4.toString(), '2');
    let esArr5 = esArr3.splice(1, 0, 'a');
    arktest.assertEQ(esArr3.toString(), '1,a,3');
    arktest.assertEQ(esArr5.toString(), '');
    return true;
}

function testToSpliced(): boolean {
    let esArr1 = createEmptyArray<string>();
    esArr1.push('1');
    esArr1.push('2');
    esArr1.push('3');
    let esArr2 = esArr1.toSpliced(1);
    arktest.assertEQ(esArr1.toString(), '1,2,3');
    arktest.assertEQ(esArr2.toString(), '1');
    let esArr4 = esArr1.toSpliced(1, 1);
    arktest.assertEQ(esArr1.toString(), '1,2,3');
    arktest.assertEQ(esArr4.toString(), '1,3');
    let esArr5 = esArr1.toSpliced(1, 1, 'a');
    arktest.assertEQ(esArr1.toString(), '1,2,3');
    arktest.assertEQ(esArr5.toString(), '1,a,3');
    return true;
}

export class User {
    private name_: string;
    private age_: int;

    constructor(name: string, age: int) {
        this.name_ = name;
        this.age_ = age;
    }

    get name(): string {
        return this.name_;
    }
    get age(): int {
        return this.age_;
    }
}

function testReferenceArray(): boolean {
    let esArr1 = createEmptyArray<object>();
    esArr1.push(new User('user1', 10));
    esArr1.push(new User('user2', 20));
    arktest.assertTrue(testUserArrayName(esArr1, 0, 'user1'));
    arktest.assertTrue(testUserArrayName(esArr1, 1, 'user2'));
    arktest.assertTrue(testUserArrayAge(esArr1, 0, 10));
    arktest.assertTrue(testUserArrayAge(esArr1, 1, 20));
    let esArr2 = returnESUserArray();
    arktest.assertEQ(esArr2[0]?.name, 'user3');
    arktest.assertEQ(esArr2[0]?.age, 11);
    arktest.assertEQ(esArr2[1]?.name, 'user4');
    arktest.assertEQ(esArr2[1]?.age, 22);
    return true;
}

function testFilter(): boolean {
    let esArr1 = createEmptyArray<number>();
    esArr1.push(1);
    esArr1.push(3);
    esArr1.push(5);
    esArr1.push(2);
    esArr1.push(4);
    esArr1.push(6);
    let esArr2 = esArr1.filter((val: number) => {return val >= 3;});
    arktest.assertEQ(esArr1.toString(), '1,3,5,2,4,6');
    arktest.assertEQ(esArr2.toString(), '3,5,4,6');
    return true;
}

function testConcat(): boolean {
    let esArr1 = getOrderArray(3);
    let esArr2 = createEmptyArray<number>();
    esArr2.push(4);
    esArr2.push(5);
    esArr2.push(6);
    let esArr3 = esArr1.concat(esArr2);
    let esArr4 = esArr1.concat(4);
    arktest.assertEQ(esArr1.toString(), '1,2,3');
    arktest.assertEQ(esArr3.toString(), '1,2,3,4,5,6');
    arktest.assertEQ(esArr4.toString(), '1,2,3,4');
    return true;
}

function testAt(): boolean {
    let esArr1 = getOrderArray(3);
    arktest.assertEQ(esArr1.at(0), 1);
    arktest.assertEQ(esArr1.at(1), 2);
    arktest.assertEQ(esArr1.at(2), 3);
    arktest.assertEQ(esArr1.at(-1), esArr1.at(2));
    arktest.assertEQ(esArr1.at(-2), esArr1.at(1));
    arktest.assertEQ(esArr1.at(-3), esArr1.at(0));
    arktest.assertTrue(esArr1.at(-4) === undefined);
    arktest.assertTrue(esArr1.at(3) === undefined);
    return true;
}

function testCopyWithin(): boolean {
    let esArr1 = getOrderArray(5);
    esArr1.copyWithin(-2);
    arktest.assertEQ(esArr1.toString(), '1,2,3,1,2');
    let esArr2 = getOrderArray(5);
    esArr2.copyWithin(0, 3);
    arktest.assertEQ(esArr2.toString(), '4,5,3,4,5');
    let esArr3 = getOrderArray(5);
    esArr3.copyWithin(0, 3, 4);
    arktest.assertEQ(esArr3.toString(), '4,2,3,4,5');
    let esArr4 = getOrderArray(5);
    esArr4.copyWithin(-2, -3, -1);
    arktest.assertEQ(esArr4.toString(), '1,2,3,3,4');
    return true;
}

function testFill(): boolean {
    let esArr1 = getOrderArray(5);
    esArr1.fill(6);
    arktest.assertEQ(esArr1.toString(), '6,6,6,6,6');
    let esArr2 = getOrderArray(5);
    esArr2.fill(6, 2);
    arktest.assertEQ(esArr2.toString(), '1,2,6,6,6');
    let esArr3 = getOrderArray(5);
    esArr3.fill(6, 1, 4);
    arktest.assertEQ(esArr3.toString(), '1,6,6,6,5');
    return true;
}

function testFind(): boolean {
    let esArr1 = getOrderArray(5);
    let result1 = esArr1.find((val: number) => { return val > 3;});
    arktest.assertEQ(result1, 4);
    let result2 = esArr1.find((val: number) => { return val > 5;});
    arktest.assertTrue(result2 === undefined);
    return true;
}

function testFindLast(): boolean {
    let esArr1 = getOrderArray(5);
    let result1 = esArr1.findLast((val: number) => { return val < 3;});
    arktest.assertEQ(result1, 2);
    let result2 = esArr1.findLast((val: number) => { return val < -1;});
    arktest.assertTrue(result2 === undefined);
    return true;
}

function testFindIndex(): boolean {
    let esArr1 = getOrderArray(5);
    let result1 = esArr1.findIndex((val: number) => { return val > 3;});
    arktest.assertEQ(result1, 3);
    let result2 = esArr1.findIndex((val: number) => { return val > 5;});
    arktest.assertEQ(result2, -1);
    return true;
}

function testFindLastIndex(): boolean {
    let esArr1 = getOrderArray(5);
    let result1 = esArr1.findLastIndex((val: number) => { return val < 3;});
    arktest.assertEQ(result1, 1);
    let result2 = esArr1.findLastIndex((val: number) => { return val < -1;});
    arktest.assertEQ(result2, -1);
    return true;
}

function testEvery(): boolean {
    let esArr1 = getOrderArray(5);
    let result1 = esArr1.every((val: number) => { return val < 3;});
    arktest.assertTrue(!result1);
    let result2 = esArr1.every((val: number) => { return val < 6;});
    arktest.assertTrue(result2);
    return true;
}

function testSome(): boolean {
    let esArr1 = getOrderArray(5);
    let result1 = esArr1.some((val: number) => { return val < 3;});
    arktest.assertTrue(result1);
    let result2 = esArr1.some((val: number) => { return val > 6;});
    arktest.assertTrue(!result2);
    return true;
}

function testReduce(): boolean {
    let esArr1 = getOrderArray(5);
    let result1 = esArr1.reduce((v1: number, v2: number) => { return v1 + v2;});
    arktest.assertEQ(result1, 15);
    let result2 = esArr1.reduce((v1: number, v2: number) => { return v1 + v2;}, 10);
    arktest.assertEQ(result2, 25);
    return true;
}

function testReduceRight(): boolean {
    let esArr1 = getOrderArray(5);
    let result1 = esArr1.reduceRight((v1: number, v2: number) => { return v1 + v2;});
    arktest.assertEQ(result1, 15);
    let result2 = esArr1.reduceRight((v1: number, v2: number) => { return v1 + v2;}, 10);
    arktest.assertEQ(result2, 25);
    return true;
}

function testSlice(): boolean {
    let esArr1 = getOrderArray(5);
    let esArr2 = esArr1.slice(2);
    arktest.assertEQ(esArr2.toString(), '3,4,5');
    arktest.assertEQ(esArr1.toString(), '1,2,3,4,5');
    let esArr3 = esArr1.slice(2, 4);
    arktest.assertEQ(esArr3.toString(), '3,4');
    let esArr4 = esArr1.slice(1, 5);
    arktest.assertEQ(esArr4.toString(), '2,3,4,5');
    let esArr5 = esArr1.slice(-2);
    arktest.assertEQ(esArr5.toString(), '4,5');
    let esArr6 = esArr1.slice(2, -1);
    arktest.assertEQ(esArr6.toString(), '3,4');
    let esArr7 = esArr1.slice();
    arktest.assertEQ(esArr7.toString(), '1,2,3,4,5');
    return true;
}

function testIndexOf(): boolean {
    let esArr1 = getOrderArray(5);
    esArr1.push(3);
    let index1 = esArr1.indexOf(3);
    arktest.assertEQ(index1, 2);
    let index2 = esArr1.indexOf(3, 3);
    arktest.assertEQ(index2, 5);
    let index3 = esArr1.indexOf(6);
    arktest.assertEQ(index3, -1);
    return true;
}

function testLastIndexOf(): boolean {
    let esArr1 = getOrderArray(5);
    esArr1.push(3);
    let index1 = esArr1.lastIndexOf(3);
    arktest.assertEQ(index1, 5);
    let index2 = esArr1.lastIndexOf(3, 4);
    arktest.assertEQ(index2, 2);
    let index3 = esArr1.lastIndexOf(6);
    arktest.assertEQ(index3, -1);
    return true;
}

function testIncludes(): boolean {
    let esArr1 = getOrderArray(5);
    let isIncludes1 = esArr1.includes(1);
    arktest.assertTrue(isIncludes1);
    let isIncludes2 = esArr1.includes(1, 1);
    arktest.assertTrue(!isIncludes2);
    return true;
}

function testJoin(): boolean {
    let esArr1 = getOrderArray(5);
    let result1 = esArr1.join();
    arktest.assertEQ(result1, '1,2,3,4,5');
    let result2 = esArr1.join('$');
    arktest.assertEQ(result2, '1$2$3$4$5');
    return true;
}

function testReverse(): boolean {
    let esArr1 = getOrderArray(5);
    let esArr2 = esArr1.reverse();
    arktest.assertEQ(esArr2.toString(), '5,4,3,2,1');
    return true;
}

function testToReversed(): boolean {
    let esArr1 = getOrderArray(5);
    let esArr2 = esArr1.toReversed();
    arktest.assertTrue(esArr2 !== esArr1);
    arktest.assertEQ(esArr1.toString(), '1,2,3,4,5');
    arktest.assertEQ(esArr2.toString(), '5,4,3,2,1');
    return true;
}

function testWith(): boolean {
    let esArr1 = getOrderArray(5);
    let esArr2 = esArr1.with(2, 6);
    arktest.assertTrue(esArr2 !== esArr1);
    arktest.assertEQ(esArr1.toString(), '1,2,3,4,5');
    arktest.assertEQ(esArr2.toString(), '1,2,6,4,5');
    return true;
}

function testMap(): boolean {
    let esArr1 = getOrderArray(5);
    let esArr2 = esArr1.map((val: number) => {return val;});
    arktest.assertTrue(esArr2 !== esArr1);
    arktest.assertEQ(esArr1.toString(), '1,2,3,4,5');
    arktest.assertEQ(esArr2.toString(), '1,2,3,4,5');
    return true;
}

function consumeESArray(val: es.Array<number>): boolean {
    arktest.assertEQ(val.toString(), '1,2,3,4,5');
    return true;
}

function testESArrayEtsToTs(): boolean {
    return esFuncArrayTest(getOrderArray(5), consumeESArray);
}

function testForEach(): boolean {
    let esArr1 = getOrderArray(5);
    let total = 0;
    let index = 0;
    esArr1.forEach((val: number, idx: number, arr: es.Array<number>) => {
        total += val;
        arktest.assertEQ(idx, index++);
        arktest.assertEQ(arr[idx], val);
    });
    arktest.assertEQ(total, 15);
    return true;
}

function testSameESArray(): boolean {
    let esArr1 = returnSameArray();
    let esArr2 = returnSameArray();
    arktest.assertTrue(esArr2 === esArr1);
    return true;
}

function testFlat(): boolean {
    let esArr1 = returnFlatArray();
    arktest.assertEQ(typeof esArr1[0], 'number');
    arktest.assertEQ(esArr1[0], 1);
    arktest.assertEQ(typeof esArr1[1], 'object');
    let esArr2 = esArr1[1];
    let esArr4 = esArr1.flat<Any>();
    arktest.assertEQ(typeof esArr4[0], 'number');
    arktest.assertEQ(typeof esArr4[1], 'number');
    arktest.assertEQ(typeof esArr4[2], 'number');
    arktest.assertEQ(typeof esArr4[3], 'object');
    let esArr5 = esArr1.flat<Any>(2);
    arktest.assertEQ(typeof esArr5[0], 'number');
    arktest.assertEQ(typeof esArr5[1], 'number');
    arktest.assertEQ(typeof esArr5[2], 'number');
    arktest.assertEQ(typeof esArr5[3], 'number');
    arktest.assertEQ(typeof esArr5[4], 'number');
    return true;
}

function testESValueConsumeESArray(): boolean {
    let module = ESValue.load('testDecl');
    let esFuncArrayTestFunc = module.getProperty('esFuncArrayTest');
    let wrappedArray = ESValue.wrap(getOrderArray(5));
    let wrappedCallback = ESValue.wrap(consumeESArray as Object);
    let result = esFuncArrayTestFunc.invoke(wrappedArray, wrappedCallback);
    return result.toBoolean();
}

function testESValueReturnESArray(): boolean {
    let module = ESValue.load('testDecl');
    let returnArray = module.getProperty('returnSameArray');
    let result = returnArray.invoke();
    let arr: es.Array<number> = result.unwrap() as es.Array<number>;
    return arr.length === 0;
}

function testAll(): void {
    testGetLength();
    testGetSetByIndex();
    testPopPush();
    testShiftUnshift();
    getOrderArray(5);
    testToString();
    testSort();
    testToSorted();
    testSplice();
    testToSpliced();
    testReferenceArray();
    testFilter();
    testConcat();
    testAt();
    testCopyWithin();
    testFill();
    testFind();
    testFindLast();
    testFindIndex();
    testFindLastIndex();
    testEvery();
    testSome();
    testReduce();
    testReduceRight();
    testSlice();
    testIndexOf();
    testLastIndexOf();
    testIncludes();
    testJoin();
    testReverse();
    testToReversed();
    testWith();
    testMap();
    testESArrayEtsToTs();
    testForEach();
    testSameESArray();
    testFlat();
    testESValueConsumeESArray();
    testESValueReturnESArray();
}