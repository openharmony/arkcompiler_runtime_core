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

export function createAndPopulateMap(): Map<string, double> {
    let map = new Map<string, double>();
    map.set('apple', 1);
    map.set('banana', 2);
    return map;
}

export function getMapSize(map: Map<string, double>): double {
    return map.size;
}

export function setToMap(map: Map<string, double>, key: string, value: double): Map<string, double> {
    map.set(key, value);
    return map;
}

export function getFromMap(map: Map<string, double>, key: string): double | undefined {
    return map.get(key);
}

export function hasInMap(map: Map<string, double>, key: string): boolean {
    return map.has(key);
}

export function deleteFromMap(map: Map<string, double>, key: string): boolean {
    return map.delete(key);
}

export function clearMap(map: Map<string, double>): void {
    map.clear();
}

export function forEachMap(map: Map<string, double>): double {
    let count = 0;
    map.forEach((value: double, key: string, m: Map<string, double>) => {
        count++;
    });
    return count;
}

export function iterateMap(map: Map<string, double>): double {
    let count = 0;
    for (let entry of map) {
        count++;
    }
    return count;
}

export function createMapWithDuplicates(): Map<string, double> {
    let map = new Map<string, double>();
    map.set('a', 1);
    map.set('b', 2);
    map.set('a', 10);
    map.set('b', 20);
    map.set('c', 3);
    return map;
}

export function chainSet(): Map<string, double> {
    let map = new Map<string, double>();
    map.set('x', 1).set('y', 2).set('z', 3);
    return map;
}

export function getNonExistentKey(map: Map<string, double>): double | undefined {
    return map.get('nonexistent');
}

export function mapWithMultipleEntries(): Map<string, double> {
    let map = new Map<string, double>();
    for (let i = 0; i < 5; i++) {
        map.set('key' + i.toString(), i);
    }
    return map;
}

export function getMapToString(map: Map<string, double>): string {
    return map.toString();
}

export function createEmptyMapForToString(): Map<string, double> {
    let map = new Map<string, double>();
    return map;
}

export function getFromMapWithDefault(map: Map<string, double>, key: string, defaultValue: double): double {
    return map.get(key, defaultValue);
}

export function getExistingKeyWithDefault(map: Map<string, double>, key: string, defaultValue: double): double {
    return map.get(key, defaultValue);
}
