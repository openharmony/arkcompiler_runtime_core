/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: >-
    The expression must be of array type (see Array Types),
    tuple type (see Tuple Types),
    or any type which has the iterator defined (see Iterable Types).
---*/

class AIterator<Double> implements IterableIterator<Double> {
    private base: A<Double>
    private index: int = 0

    constructor(base: A<Double>) {
        this.base = base
    }

    override next(): IteratorResult<Double> {
        if (this.index >= this.base.length) {
            return new IteratorResult<Double>()
        }
        return new IteratorResult<Double>(this.base.$_get(this.index++))
    }
}


class A<Double> implements Iterable<Double> {
    v: Double[] = [1.0 as Double, 2.0 as Double, 3.0 as Double]

    get length(): int { return 3 }

    $_get(index: int): Double { return this.v[index] }

    $_set(i: Double, val: Double): void { }

    $_iterator(): IterableIterator<Double> {
        return new AIterator<Double>(this)
    }
}

function display(...p: Double[]) { console.log (p) }

function main(): void {
    let a = new A<Double>
    for (let i of a) {
        console.log(i)
    }
    display(...(new A<Double>))
}
