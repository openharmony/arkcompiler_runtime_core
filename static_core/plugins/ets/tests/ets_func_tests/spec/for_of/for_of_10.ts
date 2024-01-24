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
desc: 8.9 for-of Statements. !7 - iterable
tags: []
---*/


class Fib {
    private a = 0
    private b = 1;

    [Symbol.iterator]() {
        return {
            next: () => {
                const c = this.a + this.b
                this.a = this.b
                this.b = c
                return {
                    value: this.b,
                    done: false,
                }
            },
        }
    }
}

function main() {
    let expected: number[] = [1, 2, 3, 5, 8, 13, 21]
    let result: number[] = []
    const fib = new Fib()

    for (const n of fib) {
        result.push(n);
        if (n > 20) {
            break
        }
    }

    if (result.length != expected.length) {
        console.log("Hmm ... fail wrong size")
        return 1;
    }

    if (result.toString() !== expected.toString()) {
        console.log("Hmm ... wrong content");
        return 1;
    }

    console.log("Passed");
    return 0;
}

main();
