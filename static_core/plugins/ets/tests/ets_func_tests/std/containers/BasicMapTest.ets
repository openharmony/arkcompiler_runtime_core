/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

function main(): int {
    let failures: int = 0;

    failures += test(createEmptyMap(),"Create Empty Map");
    failures += test(createMapAndAdd(), "Create Map and Add single item");
    failures += test(createMapAddAndGet(), "Create Map add and get value");
    failures += test(createMapAndGetDefault(), "Create Map and get default value");
    failures += test(createMapAddReplaceAndGet(), "Create map add and replace entry");

    failures += test(createMapAddAndClear(), "Create Map add and clear it");
    failures += test(createMapAddAndRemove(), "Create Map add and remove some key");

    failures += test(createMapAndGetKeys(), "Get All Keys from empty map");
    failures += test(createMapAddAndGetKeys(), "Get All Keys from non empty map");
    failures += test(createMapAndGetEntries(), "Get Entries from empty map");
    failures += test(createMapAddAndGetEntries(), "Get Entries from non empty map");

    failures += test(createMapAddRemoveAndGetKeys(), "Get keys after some key removed");
    failures += test(createMapAddClearAndGetKeys(), "Get keys after all keys removed");

    failures += test(createMapAddRemoveAndGetEntries(), "Get Entries after some key removed");
    failures += test(createMapAddClearAndGetEntries(), "Get Entries after all keys removed");

    failures += test(createMapAddAndCheckHasKey(),"Check map has given key");

    failures += test(createMapAddAndCheckEntriesOrder(), "Check entries order");
    failures += test(createMapAddAndCheckKeysOrder(), "Check keys order");
    return test(failures, "All tests run");
}

function getIteratorLen<T>(iter: Iterator<T>): int {
    let size: int = 0
    while(!iter.next().done)
        size++

    return size;
}

function createMapAddAndCheckEntriesOrder(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'c', 1.0);
    map.set(c'x', 2.0);
    map.set(c'a', 3.0);
    let entries = map.entries();

    if (entries.next().value![0] as Char as char == c'c' &&
        entries.next().value![0] as Char as char == c'x' &&
        entries.next().value![0] as Char as char == c'a') {
        return 0;
    }

    return 1;
}

function createMapAddAndCheckKeysOrder(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'c', 1.0);
    map.set(c'x', 2.0);
    map.set(c'a', 3.0);
    let keys = map.keys();

    if (keys.next().value == c'c' &&
        keys.next().value == c'x' &&
        keys.next().value == c'a') {
        return 0;
    }

    return 1;
}

function createMapAddAndCheckHasKey(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'a', 1.0);
    map.set(c'b', 2.0);
    map.set(c'c', 3.0);
    let result = map.has(c'c') ? 0 : 1;
    return result;
}

function createEmptyMap(): int {
    let map: Map<char, double> = new Map<char, double>();
    return map.size as int;
}

function createMapAddAndGet(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'a', 1.0);
    let expected: double = map.get(c'a')!;

    let result = expected == 1.0 ? 0 : 1;
    return result;
}

function createMapAndGetMissed(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'b', 1.0);
    try {
        map.get(c'a');
    } catch(e: Error) {
        return 0;
    }
    return 1;
}


function createMapAddReplaceAndGet(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'a', 1.0);
    map.set(c'a', 5.0);
    let expected: double = map.get(c'a')!;
    let result = expected == 5.0 ? 0 : 1;
    return result;
}

function createMapAddReplaceAndGetMissed(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'b', 1.0);
    map.set(c'b', 5.0);

    try {
        map.get(c'a');
    } catch(e: Error) {
        return 0;
    }
    return 1;
}

function createMapAndGetKeys(): int {
    let map: Map<char, double> = new Map<char, double>();
    let keys = map.keys();
    let result = map.keys().next().done ? 0 : 1;
    return result;
}

function createMapAddAndGetKeys(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'a', 1.0);
    map.set(c'c', 3.0);
    map.set(c'b', 2.0);
    let keys = map.keys();

    let result = getIteratorLen<char>(keys) == 3 ? 0 : 1;
    return result;
}

function createMapAddRemoveAndGetKeys(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'a', 1.0);
    map.set(c'b', 2.0);
    map.set(c'c', 3.0);
    map.delete(c'c');
    let keys = map.keys();
    let result = getIteratorLen<char>(keys) == 2 ? 0 : 1;
    return result;
}

function createMapAddClearAndGetKeys(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'a', 1.0);
    map.set(c'b', 2.0);
    map.set(c'c', 3.0);
    map.clear();
    let keys = map.keys();

    return getIteratorLen<char>(keys);
}

function createMapAndGetEntries(): int {
    let map: Map<char, double> = new Map<char, double>();
    let entries = map.entries();
    return getIteratorLen<[char, double]>(entries);
}

function createMapAddAndGetEntries(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'a', 1.0);
    map.set(c'b', 2.0);
    map.set(c'c', 3.0);

    let entries = map.entries();

    let result = getIteratorLen<[char, double]>(entries) == 3 ? 0 : 1;
    return result;
}


function createMapAddRemoveAndGetEntries(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'a', 1.0);
    map.set(c'b', 2.0);
    map.set(c'c', 3.0);
    map.delete(c'b');
    let entries = map.entries();

    let result = getIteratorLen<[char, double]>(entries) == 2 ? 0 : 1;
    return result;
}

function createMapAddClearAndGetEntries(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'a', 1.0);
    map.set(c'b', 2.0);
    map.set(c'c', 3.0);
    map.clear();
    let entries = map.entries();

    return getIteratorLen<[char, double]>(entries);
}

function createMapAddAndRemove(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'a', 1.0);
    map.set(c'b', 2.0);
    map.set(c'c', 3.0);
    map.delete(c'c');
    let result = map.size == 2 ? 0 : 1;
    return result;
}


function createMapAndGetDefault(): int {
    let map: Map<char, double> = new Map<char, double>();
    let expected: Double = map.get(c'a', 1.0);
    let result = expected == 1.0 ? 0 : 1;
    return result;
}

function createMapAddAndClear(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'a', 1.0);
    map.set(c'b', 2.0);
    map.set(c'c', 3.0);
    map.clear();

    return map.size as int;
}

function createMapAndAdd(): int {
    let map: Map<char, double> = new Map<char, double>();
    map.set(c'a', 1.0);
    let result = map.size == 1 ? 0 : 1;
    return result;
}

function test(result: int, message: String ): int {
    if(result == 0) {
      console.println("PASSED: " + message);
      return 0;
    }
    console.println("FAILED: " + message);
    return 1;
}
