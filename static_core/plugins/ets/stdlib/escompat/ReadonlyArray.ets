/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

package escompat;

export interface ReadonlyArray<out T> extends ConcatArray<T> {
    // concat(...items: (T | ConcatArray<T>)[]): T[];
    // concat(...items: (T | ConcatArray<T>)[]): Array<T>;
    concat(...items: FixedArray<ConcatArray<T>>): Array<T>;
    // Typing and previous overload have to cover it

    entries(): IterableIterator<[number, T]>;

    every(predicate: (value: T, index: number, array: ReadonlyArray<T>) => boolean): boolean;

    filter(predicate: (value: T, index: number, array: ReadonlyArray<T>) => boolean): Array<T>;

    find(predicate: (value: T, index: number, obj: ReadonlyArray<T>) => boolean): T | undefined;

    findLast(predicate: (value: T, index: number, obj: ReadonlyArray<T>) => boolean): T | undefined;

    findIndex(predicate: (value: T, index: number, obj: ReadonlyArray<T>) => boolean): number;

    findLastIndex(predicate: (value: T, index: number, obj: ReadonlyArray<T>) => boolean): number;

    // NOTE(ivan-tyulyandin): TBD, FlatArray is non-subset type
    // flat<A, D extends number = 1>(
    //     this: A,
    //     depth?: D,
    // ): FlatArray<A, D>[];

    // NOTE(ivan-tyulyandin): TBD
    // flatMap<U, This = undefined>(
    //     callback: (this: This, value: T, index: number, array: T[]) => U | ReadonlyArray<U>,
    //     thisArg?: This,
    // ): U[];

    forEach(action: (value: T, index: number, array: ReadonlyArray<T>) => void): void;

    join(separator?: string): string;

    includes(searchElement: T, fromIndex?: number): boolean;

    indexOf(searchElement: T, fromIndex?: number): number;

    keys(): IterableIterator<number>;

    lastIndexOf(searchElement: T, fromIndex?: number): number;

    map<U>(mapper: (value: T, index: number, array: ReadonlyArray<T>) => U): Array<U>;

    reduce(reducer: (previousValue: T, currentValue: T, currentIndex: number, array: ReadonlyArray<T>) => T): T;

    reduce<U>(reducer: (previousValue: U, currentValue: T, currentIndex: number, array: ReadonlyArray<T>) => U, initialValue: U): U;

    reduceRight(reducer: (previousValue: T, currentValue: T, currentIndex: number, array: ReadonlyArray<T>) => T): T;

    reduceRight<U>(reducer: (previousValue: U, currentValue: T, currentIndex: number, array: ReadonlyArray<T>) => U, initialValue: U): U;

    slice(start?: number, end?: number): Array<T>;

    some(predicate: (value: T, index: number, array: ReadonlyArray<T>) => boolean): boolean;

    // toLocaleString(): string;
    // All objects in ArkTS have such method, no need to define it here

    // toString(): string;
    // All objects in ArkTS have such method, no need to define it here

    values(): IterableIterator<T>;
}
