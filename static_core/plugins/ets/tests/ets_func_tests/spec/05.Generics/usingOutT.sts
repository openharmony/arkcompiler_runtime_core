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
desc: 05.Generics
name: 05.generics/usingOutT
tags: []
---*/

class A {
    returnHello1(): string {
        return "hello 1";
    }
}

class B extends A {
    override returnHello1(): string {
        return "hello 1";
    }

    returnHello2(): string {
        return "hello 2";
    }
}

class TestIterator<out T> {
    private array: T[];
    constructor(array: T[]) {
        this.array = array;
    }

    getFirstElement(): T {
        return this.array[0];
    }

    getAll(): T[] {
        return this.array;
    }
}

function main(): void {
    let a_array = [new A(), new A()];
    let iteratorA = new TestIterator<A>(a_array);
    let firstElementA: A = iteratorA.getFirstElement();
    assert(firstElementA instanceof A);
    assert(firstElementA.returnHello1() == "hello 1");

    let b_array = [new B(), new B()];
    let iteratorB = new TestIterator<B>(b_array);
    let firstElementB: B = iteratorB.getFirstElement();
    assert(firstElementB instanceof B);
    assert(firstElementB.returnHello1() == "hello 1");
    assert(firstElementB.returnHello2() == "hello 2");

    assert(a_array == iteratorA.getAll());
    assert(b_array == iteratorB.getAll());
}
