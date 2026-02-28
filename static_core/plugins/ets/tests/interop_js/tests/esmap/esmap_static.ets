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

import { es, ESUser, returnESUserMap, testUserMapName, testUserMapAge,
        esFuncMapTest, returnSameMap, createEmptyMap } from 'testDecl';

function testGetSize(): boolean {
    let esMap1: es.Map<string, number> = createEmptyMap<string, number>();
    arktest.assertEQ(esMap1.size, 0);
    return true;
}

function testSetGetHas(): boolean {
    let esMap1: es.Map<string, number> = createEmptyMap<string, number>();
    arktest.assertTrue(!esMap1.has('key1'));
    esMap1.set('key1', 100);
    esMap1.set('key2', 200);
    arktest.assertEQ(esMap1.size, 2);
    arktest.assertTrue(esMap1.has('key1'));
    arktest.assertTrue(esMap1.has('key2'));
    arktest.assertTrue(!esMap1.has('key3'));
    arktest.assertEQ(esMap1.get('key1'), 100);
    arktest.assertEQ(esMap1.get('key2'), 200);
    arktest.assertTrue(esMap1.get('key3') === undefined);
    return true;
}

function testSetUpdateValue(): boolean {
    let esMap1: es.Map<string, number> = createEmptyMap<string, number>();
    esMap1.set('key1', 100);
    arktest.assertEQ(esMap1.get('key1'), 100);
    esMap1.set('key1', 200);
    arktest.assertEQ(esMap1.size, 1);
    arktest.assertEQ(esMap1.get('key1'), 200);
    return true;
}

function testDelete(): boolean {
    let esMap1: es.Map<string, number> = createEmptyMap<string, number>();
    esMap1.set('key1', 100);
    esMap1.set('key2', 200);
    esMap1.set('key3', 300);
    arktest.assertEQ(esMap1.size, 3);
    arktest.assertTrue(esMap1.delete('key2'));
    arktest.assertEQ(esMap1.size, 2);
    arktest.assertTrue(!esMap1.has('key2'));
    arktest.assertTrue(!esMap1.delete('nonexistent'));
    arktest.assertEQ(esMap1.size, 2);
    return true;
}

function testClear(): boolean {
    let esMap1: es.Map<string, number> = createEmptyMap<string, number>();
    esMap1.set('key1', 100);
    esMap1.set('key2', 200);
    esMap1.set('key3', 300);
    arktest.assertEQ(esMap1.size, 3);
    esMap1.clear();
    arktest.assertEQ(esMap1.size, 0);
    arktest.assertTrue(!esMap1.has('key1'));
    arktest.assertTrue(!esMap1.has('key2'));
    arktest.assertTrue(!esMap1.has('key3'));
    return true;
}

function getOrderedMap(count: int): es.Map<string, number> {
    let esMap1: es.Map<string, number> = createEmptyMap<string, number>();
    for (let i = 1; i <= count; i++) {
        esMap1.set('key' + i, i * 10);
    }
    return esMap1;
}

function testToString(): boolean {
    let esMap1 = getOrderedMap(3);
    let str = esMap1.toString();
    arktest.assertTrue(str.includes('[object Map]') || str.includes('key') || str.includes('10') || str.includes('20') || str.includes('30'));
    return true;
}

function testForEach(): boolean {
    let esMap1 = getOrderedMap(3);
    let sum: number = 0;
    let count: number = 0;
    esMap1.forEach((val: number, key: string, map: es.Map<string, number>) => {
        sum += val;
        count++;
    });
    arktest.assertEQ(sum, 60);
    arktest.assertEQ(count, 3);
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

function testReferenceMap(): boolean {
    let esMap1 = createEmptyMap<string, object>();
    let user1 = new User('user1', 10);
    let user2 = new User('user2', 20);
    esMap1.set('user1', user1);
    esMap1.set('user2', user2);
    arktest.assertTrue(testUserMapName(esMap1, 'user1'));
    arktest.assertTrue(testUserMapName(esMap1, 'user2'));
    arktest.assertTrue(testUserMapAge(esMap1, 10));
    arktest.assertTrue(testUserMapAge(esMap1, 20));
    let esMap2 = returnESUserMap();
    arktest.assertTrue(esMap2.size >= 2);
    return true;
}

function testSetReturnValue(): boolean {
    let esMap1: es.Map<string, number> = createEmptyMap<string, number>();
    let result = esMap1.set('key1', 100);
    arktest.assertTrue(result === esMap1);
    esMap1.set('key2', 200);
    arktest.assertEQ(esMap1.size, 2);
    return true;
}

function testMixedTypes(): boolean {
    let esMap1: es.Map<string, number> = createEmptyMap<string, number>();
    esMap1.set('key1', 100);
    esMap1.set('key2', 200);
    esMap1.set('key3', 300);
    esMap1.set('key2', 250);
    arktest.assertEQ(esMap1.size, 3);
    arktest.assertTrue(esMap1.has('key1'));
    arktest.assertTrue(esMap1.has('key2'));
    arktest.assertTrue(esMap1.has('key3'));
    arktest.assertEQ(esMap1.get('key2'), 250);
    return true;
}

function testNumberKeys(): boolean {
    let esMap1: es.Map<number, string> = createEmptyMap<number, string>();
    esMap1.set(1, 'one');
    esMap1.set(2, 'two');
    esMap1.set(3, 'three');
    arktest.assertEQ(esMap1.size, 3);
    arktest.assertEQ(esMap1.get(1), 'one');
    arktest.assertEQ(esMap1.get(2), 'two');
    arktest.assertEQ(esMap1.get(3), 'three');
    return true;
}

function testUndefinedValue(): boolean {
    let esMap1: es.Map<string, number> = createEmptyMap<string, number>();
    esMap1.set('key1', 100);
    arktest.assertEQ(esMap1.size, 1);
    arktest.assertTrue(esMap1.has('key1'));
    arktest.assertTrue(!esMap1.has('key2'));
    arktest.assertEQ(esMap1.get('key1'), 100);
    arktest.assertTrue(esMap1.get('key2') === undefined);
    return true;
}

function consumeESMap(val: es.Map<string, number>): boolean {
    arktest.assertEQ(val.size, 3);
    arktest.assertTrue(val.has('key1'));
    arktest.assertTrue(val.has('key2'));
    arktest.assertTrue(val.has('key3'));
    return true;
}

function testESMapEtsToTs(): boolean {
    let esMap1: es.Map<string, number> = createEmptyMap<string, number>();
    esMap1.set('key1', 10);
    esMap1.set('key2', 20);
    esMap1.set('key3', 30);
    return esFuncMapTest(esMap1, consumeESMap);
}

function testSameESMap(): boolean {
    let esMap1 = returnSameMap();
    let esMap2 = returnSameMap();
    arktest.assertTrue(esMap2 === esMap1);
    return true;
}

function testEmptyMapOperations(): boolean {
    let esMap1: es.Map<string, number> = createEmptyMap<string, number>();
    arktest.assertEQ(esMap1.size, 0);
    arktest.assertTrue(!esMap1.has('anything'));
    arktest.assertTrue(!esMap1.delete('anything'));
    arktest.assertTrue(esMap1.get('anything') === undefined);
    esMap1.clear();
    arktest.assertEQ(esMap1.size, 0);
    return true;
}

function testMapChaining(): boolean {
    let esMap1: es.Map<string, number> = createEmptyMap<string, number>();
    esMap1.set('key1', 100).set('key2', 200).set('key3', 300);
    arktest.assertEQ(esMap1.size, 3);
    arktest.assertTrue(esMap1.has('key1'));
    arktest.assertTrue(esMap1.has('key2'));
    arktest.assertTrue(esMap1.has('key3'));
    return true;
}

function testLargeMap(): boolean {
    let esMap1: es.Map<string, number> = createEmptyMap<string, number>();
    for (let i = 0; i < 100; i++) {
        esMap1.set('key' + i, i);
    }
    arktest.assertEQ(esMap1.size, 100);
    for (let i = 0; i < 100; i++) {
        arktest.assertTrue(esMap1.has('key' + i));
        arktest.assertEQ(esMap1.get('key' + i), i);
    }
    return true;
}

function testAll(): void {
    testGetSize();
    testSetGetHas();
    testSetUpdateValue();
    testDelete();
    testClear();
    getOrderedMap(3);
    testToString();
    testForEach();
    testSetReturnValue();
    testMixedTypes();
    testNumberKeys();
    testUndefinedValue();
    testEmptyMapOperations();
    testMapChaining();
    testReferenceMap();
    testESMapEtsToTs();
    testLargeMap();
    testSameESMap();
}
