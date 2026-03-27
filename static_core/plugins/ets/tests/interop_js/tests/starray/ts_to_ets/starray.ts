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

namespace st {
    interface Array<T> {

        get length(): number;

        set length(newLength: number);

        [index: number]: T;

        sort(comparator?: (a: T, b: T) => number): Array<T>;

        shift(): T | undefined;

        unshift(...val: T[]): number;

        pop(): T | undefined;

        push(...val: T[]): number;

        splice(start: number): Array<T>;

        splice(start: number, deleteCount: number, ...val: T[]): Array<T>;

        keys(): IterableIterator<number>;

        values(): IterableIterator<T>;

        entries(): IterableIterator<[number, T]>;

        [Symbol.iterator](): IterableIterator<T>;

        filter(predicate: (value: T, index: number, array: Array<T>) => boolean): Array<T>;

        flat(depth?: number): Array<T>;

        concat(...val: Array<T>[]): Array<T>;

        at(key: number): T;

        copyWithin(target: number, start?: number, end?: number): Array<T>;

        fill(value: T, start?: number, end?: number): Array<T>;

        find(predicate: (value: T, index: number, array: Array<T>) => boolean): T | undefined;

        findIndex(predicate: (value: T, index: number, array: Array<T>) => boolean): number;

        findLast(predicate: (value: T, index: number, array: Array<T>) => boolean): T | undefined;

        findLastIndex(predicate: (value: T, index: number, array: Array<T>) => boolean): number;

        every(predicate: (value: T, index: number, array: Array<T>) => boolean): boolean;

        some(predicate: (value: T, index: number, array: Array<T>) => boolean): boolean;

        reduce(callback: (previousVal: T, curVal: T, idx: number, arr: Array<T>) => T, initVal?: T): T;

        reduceRight(callback: (previousVal: T, curVal: T, idx: number, arr: Array<T>) => T, initVal?: T): T;

        forEach(callbackfn: (value: T, index: number, array: Array<T>) => void): void;

        slice(start: number, end?: number): Array<T>;

        lastIndexOf(searchElement: T, fromIndex?: number): number;

        join(sep?: string): string;

        toString(): string;

        toLocaleString(): string;

        toSpliced(start?: number, deleteCount?: number, ...items: T[]): Array<T>;

        includes(val: T, fromIndex?: number): boolean;

        indexOf(val: T, fromIndex?: number): number;

        toSorted(comparator?: (a: T, b: T) => number): Array<T>;

        reverse(): Array<T>;

        toReversed(): Array<T>;

        with(index: number, val: T): Array<T>;

        map(callbackfn: (value: T, index: number, array: Array<T>) => T): Array<T>;
    }
}

const etsVm = globalThis.gtest.etsVm;
let STValue = etsVm.STValue;
const testArrayProto = etsVm.getFunction('Ltest_starray/ETSGLOBAL;', 'testArrayProto');

export function returnArray(): st.Array<string> {
    return STValue.newSTArray();
}

export function consumeArray(arr: st.Array<string>): boolean {
    return arr[0] === '1' && arr[1] === '2';
}

export function createEmptyArray(): Array {
    return new Array();
}

function main(): void {
    let res = false;
    try {
        testArrayProto([1, 2, 3]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('Value is not assignable');
        res = res && e.message.includes('Lstd/core/Array;');
    }
    ASSERT_TRUE(res);
}
main()
