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

// Numbers
export declare let jsNumber: number;
export declare let jsByte: number;
export declare let jsShort: number;
export declare let jsInt: number;
export declare let jsLong: number;
export declare let jsFloat: number;
export declare let jsDouble: number;

// Special
export declare let jsBoolean: boolean;
export declare let jsNull: null;
export declare let jsUndefined: Any;
export declare let jsBigint: bigint;

// String
export declare let jsString: string;

// Array
export declare let jsArray: es.Array<number>;

// Maps
export declare class NumberMap {
    [key: number]: number
}
export declare let jsNumberMap: NumberMap;

export declare class StringMap {
    [key: string]: number
}
export declare let jsStringMap: StringMap;

export declare class ValueHolder
{
    // Numbers
    jsNumber: number;
    jsByte: number;
    jsShort: number;
    jsInt: number;
    jsLong: number;
    jsFloat: number;
    jsDouble: number;

    // Special
    jsBoolean : boolean;
    jsNull: null;
    jsUndefined: Any;
    jsBigint: bigint;
    jsString: string;
    jsArray: es.Array<number>;
}

export declare let jsValueHolder: ValueHolder;
