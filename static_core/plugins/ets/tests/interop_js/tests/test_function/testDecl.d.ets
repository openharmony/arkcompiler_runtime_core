/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

export declare function foo1(x: string): string

export declare class X {
    s: string
    n: number
    constructor(...nums: number[])
}

export declare class Y {
    static fooArgs(...nums: number[]): number
}

export declare function foo2(x: X): string

export declare function foo3(...args: number[]): es.Array<number>

export declare function foo4(arg1: number = 1): number

export declare function bar<T>(x: T): T

type StdArray<T> = T[];

export declare namespace es {
    export class Array<T> {
        pop(): T | undefined;
        push(val: T): number;
        get length(): number;
        $_get(index: number): T | undefined;
        $_set(index: number, val: T): void;
        at(index: number): T | undefined;
        concat(val: T): Array<T>;
        copyWithin(target: number, start?: number, end?: number): Array<T>;
        every(predicate: (value: T, index: number, array: Array<T>) => boolean): boolean;
        fill(value: T, start?: number, end?: number): Array<T>;
        filter(predicate: (value: T, index: number, array: Array<T>) => boolean): Array<T>;
        find(predicate: (value: T, index: number, array: Array<T>) => boolean): T | undefined;
        findIndex(predicate: (value: T, index: number, array: Array<T>) => boolean): number;
        findLast(predicate: (value: T, index: number, array: Array<T>) => boolean): T | undefined;
        findLastIndex(predicate: (value: T, index: number, array: Array<T>) => boolean): number;
        flat<U>(depth?: number): Array<U>;
        flatMap<U>(fn: (v: T, k: number, arr: Array<T>) => U): Array<U>;
        forEach(callbackfn: (value: T, index: number, array: Array<T>) => void, thisArg?: Object): void;
        includes(val: T, fromIndex?: number): boolean;
        indexOf(val: T, fromIndex?: number): number;
        join(sep?: String): string;
        lastIndexOf(val: T, fromIndex?: number): number;
        map<U>(callbackfn: (value: T, index: number, array: Array<T>) => U): Array<U>;
        reduce(callbackfn: (accumulator: T, currentValue: T, index: number, array: Array<T>) => T): T;
        reduce<U>(
            callbackfn: (accumulator: U, currentValue: T, index: number, array: Array<T>) => U,
            initialValue: U
        ): U;
        reduceRight(callbackfn: (accumulator: T, currentValue: T, index: number, array: Array<T>) => T): T;
        reduceRight<U>(
            callbackfn: (accumulator: U, currentValue: T, index: number, array: Array<T>) => U,
            initialValue: U
        ): U;
        reverse(): Array<T>;
        shift(): T | undefined;
        slice(start?: number, end?: number): Array<T>;
        some(predicate: (value: T, index: number, array: Array<T>) => boolean): boolean;
        sort(comparator?: (a: T, b: T) => number): Array<T>;
        splice(start: number, deleteCount?: number, ...items: StdArray<T>): Array<T>;
        toString(): string;
        toLocaleString(): string;
        toReversed(): Array<T>;
        toSorted(comparator?: (a: T, b: T) => number): Array<T>;
        toSpliced(start: number, deleteCount?: number, ...items: StdArray<T>): Array<T>;
        unshift(val: T): number;
        with(index: number, value: T): Array<T>;
    }
}