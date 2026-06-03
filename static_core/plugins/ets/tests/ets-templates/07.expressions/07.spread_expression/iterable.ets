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

/*---
desc: >-
    The expression must be of array type (see Array Types),
    tuple type (see Tuple Types),
    or any type which has the iterator defined (see Iterable Types).
---*/

class IterableArrayT<T> implements Iterable<T> {
    data: T[]
    constructor(p: T[]) {
        this.data = p
    }
    $_iterator(): Iterator<T> {
        return new ArrayIteratorT<T>(this.data)
    }
}

class ArrayIteratorT<T> implements Iterator<T> {
    data: T[]
    index = 0
    constructor(data: T[]) {
        this.data = data
    }
    next(): IteratorResult<T> {
        return this.index >= this.data.length
            ? new IteratorResult<T>()
            : new IteratorResult<T>(this.data[this.index++])
    }
}

class SelfIterableArrayT<T> implements Iterable<T>, Iterator<T> {
    data: T[]
    index = 0

    constructor(p: T[]) {
        this.data = p
    }

    $_iterator(): Iterator<T> {
        return this
    }

    next(): IteratorResult<T> {
        return this.index >= this.data.length
            ? new IteratorResult<T>()
            : new IteratorResult<T>(this.data[this.index++])
    }
}

class IterableArrayCls<T> implements Iterable<T> {
    data: Array<T>
    constructor(p: Array<T>) {
        this.data = p
    }
    $_iterator(): Iterator<T> {
        return new ArrayIteratorCls<T>(this.data)
    }
}

class ArrayIteratorCls<T> implements Iterator<T> {
    data: Array<T>
    index = 0
    constructor(data: Array<T>) {
        this.data = data
    }
    next(): IteratorResult<T> {
        return this.index >= this.data.length
            ? new IteratorResult<T>()
            : new IteratorResult<T>(this.data[this.index++])
    }
}

class SelfIterableFixedArray<T> implements Iterable<T>, Iterator<T> {
    data: FixedArray<T>
    index = 0

    constructor(p: FixedArray<T>) {
        this.data = p
    }

    $_iterator(): Iterator<T> {
        return this
    }

    next(): IteratorResult<T> {
        return this.index >= this.data.length
            ? new IteratorResult<T>()
            : new IteratorResult<T>(this.data[this.index++])
    }
}

class IterableFixedArray<T> implements Iterable<T> {
    data: FixedArray<T>
    constructor(p: FixedArray<T>) {
        this.data = p
    }
    $_iterator(): Iterator<T> {
        return new FixedArrayIteratorCls<T>(this.data)
    }
}

class FixedArrayIteratorCls<T> implements Iterator<T> {
    data: FixedArray<T>
    index = 0
    constructor(data: FixedArray<T>) {
        this.data = data
    }
    next(): IteratorResult<T> {
        return this.index >= this.data.length
            ? new IteratorResult<T>()
            : new IteratorResult<T>(this.data[this.index++])
    }
}

class RestBase {
    value: string = ""

    constructor(...parts: string[]) {
        this.value = parts[0] + parts[1]
    }
}

class RestDerived extends RestBase {
    constructor(parts: Iterable<string>) {
        super(...parts)
    }
}

class RestDelegating {
    value: string = ""

    constructor(parts: Iterable<string>) {
        this(...parts)
    }

    constructor(...parts: string[]) {
        this.value = parts[0] + parts[1]
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Spread expression with custom Iterable types")

    suite.addTest("iterable with separate ArrayIterator (T[])", () => {
        let it: Iterable<double> = new IterableArrayT<double>([1.0, 2.0, 3.0])
        let r: double[] = [...it]
        arktest.assertEQ(r.length, 3)
        arktest.assertEQ(r[0], 1.0)
        arktest.assertEQ(r[1], 2.0)
        arktest.assertEQ(r[2], 3.0)
    })

    suite.addTest("self-iterable class (T[])", () => {
        let it: Iterable<double> = new SelfIterableArrayT<double>([1.0, 2.0, 3.0])
        let r: double[] = [...it]
        arktest.assertEQ(r.length, 3)
        arktest.assertEQ(r[0], 1.0)
        arktest.assertEQ(r[1], 2.0)
        arktest.assertEQ(r[2], 3.0)
    })

    suite.addTest("iterable from inline lambda (Array<T>)", () => {
        let r: Array<double> = [
            ...((p: Array<double>): Iterable<double> => new IterableArrayCls<double>(p))([1.0, 2.0, 3.0]),
        ]
        arktest.assertEQ(r.length, 3)
        arktest.assertEQ(r[0], 1.0)
        arktest.assertEQ(r[1], 2.0)
        arktest.assertEQ(r[2], 3.0)
    })

    suite.addTest("self-iterable from inline lambda (FixedArray<T>)", () => {
        let r: double[] = [
            ...((p: FixedArray<double>): Iterable<double> => new SelfIterableFixedArray<double>(p))([
                1.0, 2.0, 3.0,
            ]),
        ]
        arktest.assertEQ(r.length, 3)
        arktest.assertEQ(r[0], 1.0)
        arktest.assertEQ(r[1], 2.0)
        arktest.assertEQ(r[2], 3.0)
    })

    suite.addTest("iterable with separate FixedArrayIterator", () => {
        let r: double[] = [
            ...((p: FixedArray<double>): Iterable<double> => new IterableFixedArray<double>(p))([1.0, 2.0, 3.0]),
        ]
        arktest.assertEQ(r[0], 1.0)
        arktest.assertEQ(r[1], 2.0)
        arktest.assertEQ(r[2], 3.0)
    })

    suite.addTest("iterable spread in explicit super constructor call", () => {
        let it: Iterable<string> = new IterableArrayT<string>(["A", "B"])
        let derived = new RestDerived(it)
        arktest.assertEQ(derived.value, "AB")
    })

    suite.addTest("iterable spread in explicit this constructor call", () => {
        let it: Iterable<string> = new IterableArrayT<string>(["A", "B"])
        let delegating = new RestDelegating(it)
        arktest.assertEQ(delegating.value, "AB")
    })

    return suite.run()
}
