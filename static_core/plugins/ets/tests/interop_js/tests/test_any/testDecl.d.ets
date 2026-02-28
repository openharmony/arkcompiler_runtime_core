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

export declare class Foo
{
    bar: number;
    baz: string;
}

export declare let fooInstance: Foo;

export declare class NumberMap {
    [key: number]: number
}
export declare let numberMapInstance: NumberMap;
export declare function functionArg0(): number;
export declare function functionArg1(arg1: number): number;
export declare function functionArg2(arg1: number, arg2: number): number;

export declare class Qux {
    methodArg0(): number;
    methodArg1(arg1: number): number;
    methodArg2(arg1: number, arg2: number): number;
}

export declare let quxInstance: Qux;

export declare class TestNew0
{
    bar: number;
    constructor();
}

export declare class TestNew1 {
    bar: number;

    constructor(_bar: number);
}

export declare class TestNew2 {
    bar: number;

    constructor(lhs: number, rhs: number);
}

export declare class Foobar {
    receivedUndefined: boolean;
    constructor(arg: Object | undefined, arg2: Object);
}

export declare let arrNum: es.Array<number>;
export declare let arrFill: es.Array<number>;
export declare let big_val: bigint;

export declare class UserAccount {
    constructor(username: string, age: number);
    get username(): string;
    set username(newUsername: string);
    get age(): number;
    set age(newAge: number);
}

export declare class TestBoolean {
    boolTrue: boolean;
    boolFalse: boolean;
    boolNull: Object | null;
}
export declare let testBooleanInstance: TestBoolean;

export declare class TestArrayOps {
    arr: es.Array<number>;
}
export declare let testArrayOpsInstance: TestArrayOps;

export declare class TestTypeChecks {
    num: number;
    str: string;
    bool: boolean;
    obj: Object;
}
export declare let testTypeChecksInstance: TestTypeChecks;

export declare class TestChainedCalls {
    value: number;
    add(n: number): TestChainedCalls;
    subtract(n: number): TestChainedCalls;
    multiply(n: number): TestChainedCalls;
    getResult(): number;
}
export declare let testChainedCallsInstance: TestChainedCalls;

export declare class TestMultipleGetters {
    get firstName(): string;
    get lastName(): string;
    get fullName(): string;
}
export declare let testMultipleGettersInstance: TestMultipleGetters;

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