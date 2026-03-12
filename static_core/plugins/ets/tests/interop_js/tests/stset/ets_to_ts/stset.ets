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

export function createAndPopulateSet(): Set<string> {
    let set = new Set<string>();
    set.add('hello');
    set.add('world');
    return set;
}

export function getSetSize(set: Set<string>): int {
    return set.size;
}

export function addToSet(set: Set<string>, value: string): Set<string> {
    set.add(value);
    return set;
}

export function hasInSet(set: Set<string>, value: string): boolean {
    return set.has(value);
}

export function deleteFromSet(set: Set<string>, value: string): boolean {
    return set.delete(value);
}

export function clearSet(set: Set<string>): void {
    set.clear();
}

export function forEachSet(set: Set<string>): int {
    let count = 0;
    set.forEach((value: string, value2: string, s: Set<string>) => {
        count++;
    });
    return count;
}

export function iterateSet(set: Set<string>): int {
    let count = 0;
    for (let value of set) {
        count++;
    }
    return count;
}

export function createSetWithDuplicates(): Set<string> {
    let set = new Set<string>();
    set.add('a');
    set.add('b');
    set.add('a');
    set.add('b');
    set.add('c');
    return set;
}

export function chainAdd(): Set<string> {
    let set = new Set<string>();
    set.add('x').add('y').add('z');
    return set;
}

export function getSetToString(set: Set<string>): string {
    return set.toString();
}

export function createEmptySetForToString(): Set<string> {
    let set = new Set<string>();
    return set;
}
