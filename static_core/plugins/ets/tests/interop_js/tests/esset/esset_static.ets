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

import { es, ESUser, returnESUserSet, testUserSetName, testUserSetAge,
        esFuncSetTest, returnSameSet, createEmptySet,
        emptySet, numberSet } from 'testDecl';

function testGetSize(): boolean {
    let esSet1: es.Set<string> = createEmptySet<string>();
    arktest.assertEQ(esSet1.size, 0);
    return true;
}

function testAddHas(): boolean {
    let esSet1: es.Set<string> = createEmptySet<string>();
    arktest.assertTrue(!esSet1.has('test'));
    esSet1.add('test1');
    esSet1.add('test2');
    arktest.assertEQ(esSet1.size, 2);
    arktest.assertTrue(esSet1.has('test1'));
    arktest.assertTrue(esSet1.has('test2'));
    arktest.assertTrue(!esSet1.has('test3'));
    return true;
}

function testAddDuplicate(): boolean {
    let esSet1: es.Set<number> = createEmptySet<number>();
    esSet1.add(1);
    esSet1.add(2);
    esSet1.add(1);
    arktest.assertEQ(esSet1.size, 2);
    arktest.assertTrue(esSet1.has(1));
    arktest.assertTrue(esSet1.has(2));
    return true;
}

function testDelete(): boolean {
    let esSet1: es.Set<string> = createEmptySet<string>();
    esSet1.add('test1');
    esSet1.add('test2');
    esSet1.add('test3');
    arktest.assertEQ(esSet1.size, 3);
    arktest.assertTrue(esSet1.delete('test2'));
    arktest.assertEQ(esSet1.size, 2);
    arktest.assertTrue(!esSet1.has('test2'));
    arktest.assertTrue(!esSet1.delete('nonexistent'));
    arktest.assertEQ(esSet1.size, 2);
    return true;
}

function testClear(): boolean {
    let esSet1: es.Set<number> = createEmptySet<number>();
    esSet1.add(1);
    esSet1.add(2);
    esSet1.add(3);
    arktest.assertEQ(esSet1.size, 3);
    esSet1.clear();
    arktest.assertEQ(esSet1.size, 0);
    arktest.assertTrue(!esSet1.has(1));
    arktest.assertTrue(!esSet1.has(2));
    arktest.assertTrue(!esSet1.has(3));
    return true;
}

function getOrderedSet(count: int): es.Set<number> {
    let esSet1: es.Set<number> = createEmptySet<number>();
    for (let i = 1; i <= count; i++) {
        esSet1.add(i);
    }
    return esSet1;
}

function testToString(): boolean {
    let esSet1 = getOrderedSet(3);
    let str = esSet1.toString();
    arktest.assertTrue(str.includes('[object Set]') || str.includes('1') || str.includes('2') || str.includes('3'));
    return true;
}

function testForEach(): boolean {
    let esSet1 = getOrderedSet(5);
    let sum: number = 0;
    let count: number = 0;
    esSet1.forEach((val: number, val2: number, set: es.Set<number>) => {
        sum += val;
        count++;
    });
    arktest.assertEQ(sum, 15);
    arktest.assertEQ(count, 5);
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

function testReferenceSet(): boolean {
    let esSet1 = createEmptySet<object>();
    let user1 = new User('user1', 10);
    let user2 = new User('user2', 20);
    esSet1.add(user1);
    esSet1.add(user2);
    arktest.assertTrue(testUserSetName(esSet1, 'user1'));
    arktest.assertTrue(testUserSetName(esSet1, 'user2'));
    arktest.assertTrue(testUserSetAge(esSet1, 10));
    arktest.assertTrue(testUserSetAge(esSet1, 20));
    let esSet2 = returnESUserSet();
    arktest.assertTrue(esSet2.size >= 2);
    return true;
}

function testAddReturnValue(): boolean {
    let esSet1: es.Set<number> = createEmptySet<number>();
    let result = esSet1.add(1);
    arktest.assertTrue(result === esSet1);
    esSet1.add(2);
    arktest.assertEQ(esSet1.size, 2);
    return true;
}

function testMixedTypes(): boolean {
    let esSet1: es.Set<number> = createEmptySet<number>();
    esSet1.add(1);
    esSet1.add(2);
    esSet1.add(3);
    esSet1.add(2);
    arktest.assertEQ(esSet1.size, 3);
    arktest.assertTrue(esSet1.has(1));
    arktest.assertTrue(esSet1.has(2));
    arktest.assertTrue(esSet1.has(3));
    return true;
}

function consumeESSet(val: es.Set<number>): boolean {
    arktest.assertEQ(val.size, 3);
    arktest.assertTrue(val.has(1));
    arktest.assertTrue(val.has(2));
    arktest.assertTrue(val.has(3));
    return true;
}

function testESSetEtsToTs(): boolean {
    let esSet1: es.Set<number> = createEmptySet<number>();
    esSet1.add(1);
    esSet1.add(2);
    esSet1.add(3);
    return esFuncSetTest(esSet1, consumeESSet);
}

function testSameESSet(): boolean {
    let esSet1 = returnSameSet();
    let esSet2 = returnSameSet();
    arktest.assertTrue(esSet2 === esSet1);
    return true;
}

function testEmptySetOperations(): boolean {
    let esSet1: es.Set<string> = createEmptySet<string>();
    arktest.assertEQ(esSet1.size, 0);
    arktest.assertTrue(!esSet1.has('anything'));
    arktest.assertTrue(!esSet1.delete('anything'));
    esSet1.clear();
    arktest.assertEQ(esSet1.size, 0);
    return true;
}

function testSetChaining(): boolean {
    let esSet1: es.Set<number> = createEmptySet<number>();
    esSet1.add(1).add(2).add(3);
    arktest.assertEQ(esSet1.size, 3);
    arktest.assertTrue(esSet1.has(1));
    arktest.assertTrue(esSet1.has(2));
    arktest.assertTrue(esSet1.has(3));
    return true;
}

function testLargeSet(): boolean {
    let esSet1: es.Set<number> = createEmptySet<number>();
    for (let i = 0; i < 100; i++) {
        esSet1.add(i);
    }
    arktest.assertEQ(esSet1.size, 100);
    for (let i = 0; i < 100; i++) {
        arktest.assertTrue(esSet1.has(i));
    }
    return true;
}

function testNewSet(): boolean {
    let global = ESValue.getGlobal();
    let setClass = global.getProperty('Set');
    let esSet1 = setClass.instantiate().unwrap() as es.Set<string>;
    arktest.assertEQ(esSet1.size, 0);
    esSet1.add('test1');
    esSet1.add('test2');
    arktest.assertEQ(esSet1.size, 2);
    arktest.assertTrue(esSet1.has('test1'));
    arktest.assertTrue(esSet1.has('test2'));
    arktest.assertTrue(esSet1.delete('test1'));
    arktest.assertTrue(esSet1.delete('test2'));
    arktest.assertTrue(!esSet1.delete('test3'));
    arktest.assertEQ(esSet1.size, 0);
    return true;
}

function testIsSet(): boolean {
    let global = ESValue.getGlobal();
    let setClass = global.getProperty('Set');
    let esSet1 = setClass.instantiate().unwrap() as es.Set<string>;
    arktest.assertTrue(ESValue.wrap(esSet1).instanceOf(setClass));
    arktest.assertTrue(ESValue.wrap(numberSet).instanceOf(setClass));
    arktest.assertTrue(ESValue.wrap(emptySet).instanceOf(setClass));

    let staticSet = new Set<string>();
    staticSet.add('test');
    arktest.assertTrue(!ESValue.wrap(staticSet).instanceOf(setClass));
    return true;
}

function testAll(): void {
    testGetSize();
    testAddHas();
    testAddDuplicate();
    testDelete();
    testClear();
    getOrderedSet(3);
    testToString();
    testForEach();
    testReferenceSet();
    testAddReturnValue();
    testMixedTypes();
    testESSetEtsToTs();
    testSameESSet();
    testEmptySetOperations();
    testSetChaining();
    testLargeSet();
    testNewSet();
    testIsSet();
}
