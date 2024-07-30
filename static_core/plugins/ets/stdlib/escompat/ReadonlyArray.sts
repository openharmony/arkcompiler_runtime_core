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

package escompat;

export interface ReadonlyArray<out T> extends ConcatArray<T> {
    // concat(...items: (T | ConcatArray<T>)[]): T[];
    // concat(...items: (T | ConcatArray<T>)[]): Array<T>;
    concat(...items: ConcatArray<T>[]): Array<T>;
    // Typing and previous overload have to cover it

    entries(): IterableIterator<[number, T]>;

    every(predicate: (value: T, index: number, array: ReadonlyArray<T>) => boolean): boolean;
    every(predicate: (value: T, index: number) => boolean): boolean;
    every(predicate: (value: T) => boolean): boolean;
    every(predicate: () => boolean): boolean;
    // every<S extends T>(predicate: (value: T, index: number, array: readonly T[]) => value is S, thisArg?: any): this is readonly S[];
    // Typing and previous overload have to cover it

    filter(predicate: (value: T, index: number, array: ReadonlyArray<T>) => boolean): Array<T>;
    filter(predicate: (value: T, index: number) => boolean): Array<T>;
    filter(predicate: (value: T) => boolean): Array<T>;
    filter(predicate: () => boolean): Array<T>;
    // filter<S extends T>(predicate: (value: T, index: number, array: readonly T[]) => value is S, thisArg?: any): S[];
    // Typing and previous overload have to cover it

    find(predicate: (value: T, index: number, obj: ReadonlyArray<T>) => boolean): T | undefined;
    find(predicate: (value: T, index: number) => boolean): T | undefined;
    find(predicate: (value: T) => boolean): T | undefined;
    find(predicate: () => boolean): T | undefined;
    // find<S extends T>(predicate: (value: T, index: number, obj: readonly T[]) => value is S, thisArg?: any): S | undefined;
    // Typing and previous overload have to cover it

    findLast(predicate: (value: T, index: number, obj: ReadonlyArray<T>) => boolean): T | undefined;
    findLast(predicate: (value: T, index: number) => boolean): T | undefined;
    findLast(predicate: (value: T) => boolean): T | undefined;
    findLast(predicate: () => boolean): T | undefined;

    findIndex(predicate: (value: T, index: number, obj: ReadonlyArray<T>) => boolean): number;
    findIndex(predicate: (value: T, index: number) => boolean): number;
    findIndex(predicate: (value: T) => boolean): number;
    findIndex(predicate: () => boolean): number;

    findLastIndex(predicate: (value: T, index: number, obj: ReadonlyArray<T>) => boolean): number;
    findLastIndex(predicate: (value: T, index: number) => boolean): number;
    findLastIndex(predicate: (value: T) => boolean): number;
    findLastIndex(predicate: () => boolean): number;

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
    forEach(callbackfn: (value: T, index: number) => void): void;
    forEach(callbackfn: (value: T) => void): void;
    forEach(callbackfn: () => void): void;

    join(separator?: string): string;

    includes(searchElement: T, fromIndex?: number): boolean;

    indexOf(searchElement: T, fromIndex?: number): number;

    keys(): IterableIterator<number>;

    lastIndexOf(searchElement: T, fromIndex?: number): number;

    map<U>(mapper: (value: T, index: number, array: ReadonlyArray<T>) => U): Array<U>;
    map<U>(callbackfn: (value: T, index: number) => U): Array<U>;
    map<U>(callbackfn: (value: T) => U): Array<U>;
    map<U>(callbackfn: () => U): Array<U>;

    reduce(reducer: (previousValue: T, currentValue: T, currentIndex: number, array: ReadonlyArray<T>) => T): T;
    reduce(callbackfn: (previousValue: T, currentValue: T, currentIndex: number) => T): T;
    reduce(callbackfn: (previousValue: T, currentValue: T) => T): T;
    reduce(callbackfn: (previousValue: T) => T): T;
    reduce(callbackfn: () => T): T;

    reduce<U>(reducer: (previousValue: U, currentValue: T, currentIndex: number, array: ReadonlyArray<T>) => U, initialValue: U): U;
    reduce<U>(callbackfn: (previousValue: U, currentValue: T, currentIndex: number) => U, initialValue: U): U;
    reduce<U>(callbackfn: (previousValue: U, currentValue: T) => U, initialValue: U): U;
    reduce<U>(callbackfn: (previousValue: U) => U, initialValue: U): U;
    reduce<U>(callbackfn: () => U, initialValue: U): U;

    reduceRight(reducer: (previousValue: T, currentValue: T, currentIndex: number, array: ReadonlyArray<T>) => T): T;
    reduceRight(callbackfn: (previousValue: T, currentValue: T, currentIndex: number) => T): T;
    reduceRight(callbackfn: (previousValue: T, currentValue: T) => T): T;
    reduceRight(callbackfn: (previousValue: T) => T): T;
    reduceRight(callbackfn: () => T): T;

    reduceRight<U>(reducer: (previousValue: U, currentValue: T, currentIndex: number, array: ReadonlyArray<T>) => U, initialValue: U): U;
    reduceRight<U>(callbackfn: (previousValue: U, currentValue: T, currentIndex: number) => U, initialValue: U): U;
    reduceRight<U>(callbackfn: (previousValue: U, currentValue: T) => U, initialValue: U): U;
    reduceRight<U>(callbackfn: (previousValue: U) => U, initialValue: U): U;
    reduceRight<U>(callbackfn: () => U, initialValue: U): U;

    slice(start?: number, end?: number): Array<T>;

    some(predicate: (value: T, index: number, array: ReadonlyArray<T>) => boolean): boolean;
    some(predicate: (value: T, index: number) => boolean): boolean;
    some(predicate: (value: T) => boolean): boolean;
    some(predicate: () => boolean): boolean;

    // toLocaleString(): string;
    // All objects in ArkTS have such method, no need to define it here

    // toString(): string;
    // All objects in ArkTS have such method, no need to define it here

    values(): IterableIterator<T>;
}
