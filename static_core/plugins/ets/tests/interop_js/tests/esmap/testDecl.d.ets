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

export declare class ESUser {
    get name(): string;
    get age(): number;
}

export declare function returnESUserMap(): es.Map<string, ESUser>;
export declare function testUserMapName(map: es.Map<string, object>, name: string): boolean;
export declare function testUserMapAge(map: es.Map<string, object>, age: number): boolean;
export declare function returnSameMap(): es.Map<string, number>;
export declare function createEmptyMap<K, V>(): es.Map<K, V>;
export declare function esFuncMapTest(
    map: es.Map<string, number>,
    func: (
        map: es.Map<string, number>
    ) => boolean
): boolean;

export declare namespace es {
    export class Map<K, V> {
        set(key: K, value: V): this;
        get(key: K): V | undefined;
        has(key: K): boolean;
        delete(key: K): boolean;
        clear(): void;
        forEach(callbackfn: (value: V, key: K, map: Map<K, V>) => void, thisArg?: Object): void;
        get size(): number;
        toString(): string;
    }
}