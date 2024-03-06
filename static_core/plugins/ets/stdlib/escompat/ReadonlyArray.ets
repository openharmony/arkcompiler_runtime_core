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
    // concat(...items: ConcatArray<T>[]): T[];
    // Typing and previous overload have to cover it

    //entries(): IterableIterator<[number, T]>;

    every(predicate: (value: T, index: number, array: ReadonlyArray<T>) => boolean): boolean { throw new Error("#15998") }
    every(predicate: (value: T, index: number) => boolean): boolean;
    every(predicate: (value: T) => boolean): boolean;
    // every<S extends T>(predicate: (value: T, index: number, array: readonly T[]) => value is S, thisArg?: any): this is readonly S[];
    // Typing and previous overload have to cover it

    filter(predicate: (value: T, index: number, array: ReadonlyArray<T>) => boolean): Array<T> { throw new Error("#15998") }
    filter(predicate: (value: T, index: number) => boolean): Array<T>;
    filter(predicate: (value: T) => boolean): Array<T>;
    // filter<S extends T>(predicate: (value: T, index: number, array: readonly T[]) => value is S, thisArg?: any): S[];
    // Typing and previous overload have to cover it

    // TODO(ivan-tyulyandin): blocked by undefined implementation
    find(predicate: (value: T, index: number, obj: ReadonlyArray<T>) => boolean): T | undefined { throw new Error("#15998") }
    find(predicate: (value: T, index: number) => boolean): T | undefined;
    find(predicate: (value: T) => boolean): T | undefined;
    // find<S extends T>(predicate: (value: T, index: number, obj: readonly T[]) => value is S, thisArg?: any): S | undefined;
    // Typing and previous overload have to cover it

    findLast(predicate: (value: T, index: number, obj: ReadonlyArray<T>) => boolean): T | undefined { throw new Error("#15998") }
    findLast(predicate: (value: T, index: number) => boolean): T | undefined;
    findLast(predicate: (value: T) => boolean): T | undefined;

    findIndex(predicate: (value: T, index: number, obj: ReadonlyArray<T>) => boolean): number { throw new Error("#15998") }
    findIndex(predicate: (value: T, index: number) => boolean): number;
    findIndex(predicate: (value: T) => boolean): number;

    findLastIndex(predicate: (value: T, index: number, obj: ReadonlyArray<T>) => boolean): number { throw new Error("#15998") }
    findLastIndex(predicate: (value: T, index: number) => boolean): number;
    findLastIndex(predicate: (value: T) => boolean): number;

    // TODO(ivan-tyulyandin): TBD, FlatArray is non-subset type
    // flat<A, D extends number = 1>(
    //     this: A,
    //     depth?: D,
    // ): FlatArray<A, D>[];

    // TODO(ivan-tyulyandin): TBD
    // flatMap<U, This = undefined>(
    //     callback: (this: This, value: T, index: number, array: T[]) => U | ReadonlyArray<U>,
    //     thisArg?: This,
    // ): U[];

    forEach(action: (value: T, index: number, array: ReadonlyArray<T>) => void): void { throw new Error("#15998") }
    forEach(callbackfn: (value: T, index: number) => void): void;
    forEach(callbackfn: (value: T) => void): void;

    includes(searchElement: T, fromIndex: Number | undefined): boolean;

    indexOf(searchElement: T, fromIndex: Number | undefined): number;

    //keys(): IterableIterator<number>;

    lastIndexOf(searchElement: T, fromIndex: Number | undefined): number;

    map<U>(mapper: (value: T, index: number, array: ReadonlyArray<T>) => U): Array<U> { throw new Error("#15998") }
    map<U>(callbackfn: (value: T, index: number) => U): Array<U>;
    map<U>(callbackfn: (value: T) => U): Array<U>;

    reduce(reducer: (previousValue: T, currentValue: T, currentIndex: number, array: ReadonlyArray<T>) => T): T {
        throw new Error("#15998")
    }
    reduce(callbackfn: (previousValue: T, currentValue: T, currentIndex: number) => T): T;
    reduce(callbackfn: (previousValue: T, currentValue: T) => T): T;

    reduce<U>(reducer: (previousValue: U, currentValue: T, currentIndex: number, array: ReadonlyArray<T>) => U, initialValue: U): U {
        throw new Error("#15998")
    }
    reduce<U>(callbackfn: (previousValue: U, currentValue: T, currentIndex: number) => U, initialValue: U): U;
    reduce<U>(callbackfn: (previousValue: U, currentValue: T) => U, initialValue: U): U;

    reduceRight(reducer: (previousValue: T, currentValue: T, currentIndex: number, array: ReadonlyArray<T>) => T): T {
        throw new Error("#15998")
    }
    reduceRight(callbackfn: (previousValue: T, currentValue: T, currentIndex: number) => T): T;
    reduceRight(callbackfn: (previousValue: T, currentValue: T) => T): T;

    reduceRight<U>(reducer: (previousValue: U, currentValue: T, currentIndex: number, array: ReadonlyArray<T>) => U, initialValue: U): U {
        throw new Error("#15998")
    }
    reduceRight<U>(callbackfn: (previousValue: U, currentValue: T, currentIndex: number) => U, initialValue: U): U;
    reduceRight<U>(callbackfn: (previousValue: U, currentValue: T) => U, initialValue: U): U;

    // some(predicate: (value: T, index: number, array: readonly T[]) => unknown, thisArg?: any): boolean;
    some(predicate: (value: T, index: number) => boolean): boolean;
    some(predicate: (value: T) => boolean): boolean;

    toLocaleString(): string;

    // toString(): string;
    // All objects in ArkTS have such method, no need to define it here

    values(): IterableIterator<T>;
}
