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
    interface Set<T> {

        get size(): number;

        add(value: T): this;

        has(value: T): boolean;

        delete(value: T): boolean;

        clear(): void;

        forEach(callbackfn: (value: T, value2: T, set: Set<T>) => void): void;

        keys(): IterableIterator<T>;

        values(): IterableIterator<T>;

        entries(): IterableIterator<[T, T]>;

        toString(): string;
        
        [Symbol.iterator](): IterableIterator<T>;

        [index: number]: T | undefined;
    }
}

const etsVm = globalThis.gtest.etsVm;
let STValue = etsVm.STValue;
const testSetProto = etsVm.getFunction('Ltest_stset/ETSGLOBAL;', 'testSetProto');

export function returnSet(): st.Set<string> {
    return STValue.newSTSet();
}

export function consumeSet(set: st.Set<string>): boolean {
    set.add('hello');
    set.add('world');
    return set.size === 2 && set.has('hello') && set.has('world');
}

export function checkUniqueness(set: st.Set<string>): boolean {
    let beforeSize = set.size;
    set.add('a');
    set.add('a');
    set.add('b');
    set.add('b');
    return set.size === beforeSize + 2;
}

export function checkSizeIsProperty(set: st.Set<string>): boolean {
    return typeof set.size === 'number';
}

export function checkInsertionOrder(set: st.Set<string>): boolean {
    let values: string[] = [];
    for (let value of set) {
        values.push(value);
    }
    return values[0] === 'first' && values[1] === 'second' && values[2] === 'third';
}

export function checkChaining(set: st.Set<string>): boolean {
    let result = set.add('x').add('y').add('z');
    return result === set && set.size === 3;
}

export function checkNoIndexAccess(set: st.Set<string>): boolean {
    return set[0] === undefined && set[1] === undefined;
}

export function checkDeleteReturnsBoolean(set: st.Set<string>): boolean {
    let added = set.add('test');
    let deleted = set.delete('test');
    let deletedNonExistent = set.delete('nonexistent');
    return typeof deleted === 'boolean' && deleted === true &&
           typeof deletedNonExistent === 'boolean' && deletedNonExistent === false;
}

export function checkHasMethod(set: st.Set<string>): boolean {
    set.add('a');
    set.add('b');
    return typeof set.has === 'function' && set.has('a') === true && set.has('c') === false;
}

export function checkClearMethod(set: st.Set<string>): void {
    set.add('a');
    set.add('b');
    set.add('c');
    set.clear();
}

export function createEmptySet(): Set<number> {
    return new Set<number>;
}

function main(): void {
    let res = false;
    try {
        testSetProto(new Set<number>);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('Value is not assignable');
        res = res && e.message.includes('Lstd/core/Set;');
    }
    ASSERT_TRUE(res);
}

main()
