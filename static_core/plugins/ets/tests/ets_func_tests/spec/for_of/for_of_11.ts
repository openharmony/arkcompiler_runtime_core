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

/*---
desc: 8.9 for-of Statements. 17.4, 17.5.1 - iterable
tags: []
---*/


class Bar {
    index : number;
    constructor(index: number) {
        this.index = index;
    }
}

class Foo<T> implements Iterable<T> {
    values: T[]
    constructor(p_values: T[]) { this.values = p_values }
    [Symbol.iterator](): FooIterator<T> {
        return new FooIterator(this.values);
    }
}

class FooIterator<T> implements Iterator<T> {
    private index: number;
    private done: boolean;
    private values: T[];

    constructor(values: T[]) {
        this.index = 0;
        this.done = false;
        this.values = values;
    }

    next(): IteratorResult<T, number | undefined> {
        if (this.done)
            return {
                done: this.done,
                value: undefined
            };

        if (this.index === this.values.length) {
            this.done = true;
            return {
                done: this.done,
                value: this.index
            };
        }

        const value = this.values[this.index];
        this.index++;
        return {
            done: false,
            value
        };
    }
}


function main() {
    let result: Bar[] = [];
    let expected: Bar[] = [new Bar(1), new Bar(2), new Bar(3)];
    const foo = new Foo<Bar>([new Bar(1), new Bar(2), new Bar(3)]);
    for (const value of foo) {
        result.push(value);
    }

    if (result.length != expected.length) {
        console.log("Hmm ... fail wrong size")
        return 1;
    }

    for (let i = 0; i < result.length; i++) {
        /* todo ETS
        if (result[i] !instanceof Bar) {
            console.log("Incorrect type at index : " + i)
            return 1;
        }
        */

        if (result[i].index != expected[i].index) {
            console.log("Incorrec content at index : " + i);
            return 1;
        }
    }

    console.log("Passed");
    return 0;
}

main();
