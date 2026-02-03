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

class MyReadonlyArray<T> implements ReadonlyArray<T> {
    private impl: Array<T>;

    constructor(source: ReadonlyArray<T>) {
        this.impl = new Array<T>();
        for (let item of source) {
            this.impl.push(item);
        }
	}

    override $_get(idx: int): T {
        return this.impl.$_get(idx);
    }

    $_set(idx: int, val: T): void {
        this.impl.$_set(idx, val);
    }

    override at(index: int): T | undefined {
        return this.impl.at(index);
    }

    override get length(): int {
        return this.impl.length;
    }

    override values(): IterableIterator<T> {
        return this.impl.values();
    }

    override $_iterator(): IterableIterator<T> {
        return this.impl.$_iterator();
    }

    override concat(...items: FixedArray<ConcatArray<T>>): Array<T> {
        throw new Error('Not used in test');
    }

    override entries(): IterableIterator<[number, T]> {
        throw new Error('Not used in test');
    }

    override every(predicate: (value: T, index: int, array: ReadonlyArray<T>) => boolean): boolean {
        throw new Error('Not used in test');
    }

    override filter(predicate: (value: T, index: int, array: ReadonlyArray<T>) => boolean): Array<T> {
        throw new Error('Not used in test');
    }

    override find(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): T | undefined {
        throw new Error('Not used in test');
    }

    override findLast(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): T | undefined {
        throw new Error('Not used in test');
    }

    override findIndex(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): int {
        throw new Error('Not used in test');
    }

    override findLastIndex(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): int {
        throw new Error('Not used in test');
    }

    override forEach(action: (value: T, index: int, array: ReadonlyArray<T>) => void): void {
        throw new Error('Not used in test');
    }

    override join(separator?: string): string {
        throw new Error('Not used in test');
    }

    override includes(searchElement: T, fromIndex?: int): boolean {
        throw new Error('Not used in test');
    }

    override indexOf(searchElement: T, fromIndex?: int): int {
        throw new Error('Not used in test');
    }

    override keys(): IterableIterator<number> {
        throw new Error('Not used in test');
    }

    override lastIndexOf(searchElement: T): int {
        throw new Error('Not used in test');
    }

    override lastIndexOf(searchElement: T, fromIndex?: int): int {
        throw new Error('Not used in test');
    }

    override map<U>(mapper: (value: T, index: int, array: ReadonlyArray<T>) => U): Array<U> {
        return this.impl.map<U>(mapper);
    }

    override reduce(reducer: (previousValue: T, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => T): T {
        throw new Error('Not used in test');
    }

    override reduce<U>(reducer: (previousValue: U, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => U, initialValue: U): U {
        throw new Error('Not used in test');
    }

    override reduceRight(reducer: (previousValue: T, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => T): T {
        throw new Error('Not used in test');
    }

    override reduceRight<U>(reducer: (previousValue: U, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => U, initialValue: U): U {
        throw new Error('Not used in test');
    }

    override slice(start?: int, end?: int): Array<T> {
        throw new Error('Not used in test');
    }

    override some(predicate: (value: T, index: int, array: ReadonlyArray<T>) => boolean): boolean {
        throw new Error('Not used in test');
    }
}

function foo(a: int | Array<int>): int | Array<int> {
    if (a instanceof Array) {
        return (a as Array<int>).map<int>((b: int): int => b * 2);
    }
    return [a as int * 2];
}

function compareArray<T>(first: Array<T>, second: Array<T>) {
    arktest.assertEQ(first.length, second.length)
    for (let i = 0; i < first.length; ++i) {
        if (first[i] instanceof Array) {
            compareArray<T>(first[i] as Array<T>, second[i] as Array<T>)
        } else {
            arktest.assertEQ(first[i], second[i])
        }
    }
}

function flatMapTest1() {
    const arr: Array<int> = [1, 2, 1]
    const result: Array<int> = arr.flatMap<int>((num: int): Array<int> => (num === 2 ? [2, 2] : [1]))
    const expected: Array<int> = [1, 2, 2, 1]
    compareArray<int>(result, expected)

    const arr1: Array<int | undefined> = [1, undefined, 1];
    const result1: Array<int> = arr1
        .flatMap<int>((num: int | undefined): Array<int> => (num === 2 ? [2, 2] : [1]));
    const expected1: Array<int> = [1, 1, 1]
    compareArray<int>(result1, expected1)
}

function flatMapTest2() {
    const arr: Array<string> = ['one two', 'three four']
    const result: Array<string> = arr.flatMap<string>((word: string): Array<string> => (word.split(' ')))
    const expected: Array<string> = ['one', 'two', 'three', 'four']
    compareArray<string>(result, expected)
}

function flatMapTest3() {
    const expected: Array<string> = ['lib.cpp', 'lib.ets', 'lib.py', 'index.cpp', 'index.ets', 'index.py']
    const exts: Array<string> = ['.cpp', '.ets', '.py']
    const files: Array<string> = ['lib', 'index']
        .flatMap<string>((entry: string): Array<string> =>
            exts.map<string>((ext: string): string => {
                let x: string = entry + ext
                return x
            }))
    compareArray<string>(files, expected)
}

function flatMapTest4() {
    const expected: Array<int> = [2, 4, 6, 8]
    const arr: Array<Array<int>> = [[1, 2], [3, 4]]
    const result: Array<int> = arr
        .flatMap<int>((x: Array<int>): Array<int> => x.map<int>((v: int): int => v * 2))
    compareArray<int>(result, expected)
}

function flatMapTest5() {
    const expected: Array<int> = [2, 4, 6, 8]
    const arr: Array<int | Array<int>> = [1, [2, 3], 4];
    const result = arr.flatMap<int>((y: int | Array<int>): int | Array<int> => foo(y));
    compareArray<int>(result, expected)
}

function flatMapTest6() {
    const expected: Array<int> = [2, 4, 3, 4, 10, 12]
    const arr: Array<int | Array<int | Array<int>>> = [1, [2, [3, 4], 5], 6];
    const result: Array<int> = arr
        .flatMap<int>((x: int | Array<int | Array<int>>): Array<int> => {
            if (x instanceof Array) {
                const y = x as Array<int | Array<int>>;
                return y.flatMap<int>((z: int | Array<int>): Array<int> => {
                    return z instanceof Array ? z as Array<int> : [z as int * 2];
                });
            }
            return [x as int * 2];
        });
    compareArray<int>(result, expected)
}

function flatMapTest7() {
    const expected: Array<int> = [2, 4, 6, 8, 10, 12]
    const arr: Array<int | Array<int | Array<int>>> = [1, [2, [3, 4], 5], 6];
    const result: Array<int> = arr
        .flatMap<int>((x: int | Array<int | Array<int>>): int | Array<int> => {
            if (x instanceof Array) {
                const y = x as Array<int | Array<int>>
                return y.flatMap<int>((z: int | Array<int>): int | Array<int> => foo(z));
            }
            return [x as int * 2];
        });
    compareArray<int>(result, expected)
}

function flatMapTest8() {
    // Test case int | ReadonlyArray
    const arr: Array<int> = [1, 2, 3, 4]
    const result: Array<int> = arr
        .flatMap<int>((num: int): int | ReadonlyArray<int> => {
            let arr = new Array<int>
            if (num > 1 && num < 4) {
                arr.push(num * 2)
                arr.push(num * 3)
                return new MyReadonlyArray<int>(arr);
            } else {
                return num;
            }
        });
    const expected: Array<int> = [1, 4, 6, 6, 9, 4]
    compareArray<int>(result, expected)
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Array.flatMap tests')
    suite.addTest('test 1', flatMapTest1)
    suite.addTest('test 2', flatMapTest2)
    suite.addTest('test 3', flatMapTest3)
    suite.addTest('test 4', flatMapTest4)
    suite.addTest('test 5', flatMapTest5)
    suite.addTest('test 6', flatMapTest6)
    suite.addTest('test 7', flatMapTest7)
    suite.addTest('test 7', flatMapTest8)
    return suite.run()
}
