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

export declare function getPromiseNumber(): Promise<number>;
export declare function getPromiseString(): Promise<string>;
export declare function getPromiseBoolean(): Promise<boolean>;
export declare function getPromiseBigInt(): Promise<bigint>;
export declare function getPromiseNull(): Promise<null>;
export declare function getPromiseUndefined(): Promise<undefined>;
export declare function getPromiseNan(): Promise<number>;
export declare function getPromiseInfinity(): Promise<number>;
export declare function getRejectPromise(): Promise<void>;
export declare function getFulfilled1(): Promise<number>;
export declare function getFulfilled2(): Promise<number>;
export declare function getFulfilled3(): Promise<number>;
export declare function getRejected1(): Promise<void>;
export declare function getRejected2(): Promise<void>;
export declare function createAllEmpty(): Promise<es.Array<Object>>;
export declare function createAllWithPromises(): Promise<es.Array<number>>;
export declare function createAllWithRejected(): Promise<es.Array<number>>;
export declare function createAllMixed(): Promise<es.Array<Object>>;
export declare function createAllWithString(): Promise<es.Array<string>>;
export declare function createAnyEmpty(): Promise<void>;
export declare function createAnyAllRejected(): Promise<es.Array<string>>;
export declare function createAnyMixed(): Promise<number>;
export declare function createAnyFirstWins(): Promise<string>;
export declare function createRaceEmpty(): Promise<void>;
export declare function createRacePlainValues(): Promise<number>;
export declare function createRaceFulfilledWins(): Promise<string>;
export declare function createRaceRejectedWins(): Promise<void>;
export declare function createRaceMixedImmediateWins(): Promise<number>;
export declare function createRaceRejectImmediate(): Promise<void>;
export declare function createRaceWithString(): Promise<string>;
export declare let cbLambda: () => Promise<number>;

export declare class CreateClass1 {
    foo(): Promise<string>;
    static foo1(): Promise<string>;
}

type StdArray<T> = T[];

// NOTE(#<10914>): create a single declaration for all tests.
export declare namespace es {
    export class Array<T> {
        pop(): T | undefined;
        push(val: T): number;
        get length(): number;
        $_get(index: number): T | undefined;
        $_set(index: number, val: T): void;
        at(index: number): T | undefined;
        concat(item: T | Array<T>): Array<T>;
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
