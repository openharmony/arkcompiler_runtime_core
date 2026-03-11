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

export declare function returnESUserSet(): es.Set<ESUser>;
export declare function testUserSetName(set: es.Set<object>, name: string): boolean;
export declare function testUserSetAge(set: es.Set<object>, age: number): boolean;
export declare function esFuncSetTest(set: es.Set<number>, func: (set: es.Set<number>) => boolean): boolean;
export declare function returnSameSet(): es.Set<number>;
export declare function createEmptySet<T>(): es.Set<T>;

export declare namespace es {
    export class Set<T> {
        add(value: T): this;
        clear(): void;
        delete(value: T): boolean;
        forEach(callbackfn: (value: T, value2: T, set: Set<T>) => void, thisArg?: Object): void;
        has(value: T): boolean;
        get size(): number;
        toString(): string;
    }
}
