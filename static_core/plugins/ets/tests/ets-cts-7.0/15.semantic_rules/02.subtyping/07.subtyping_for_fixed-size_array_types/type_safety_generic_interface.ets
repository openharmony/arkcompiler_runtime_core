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
    Fixed-size array types are covariant and are formally defined as follows:
    FixedSize<B> <: FixedSize<A> if B <: A.
---*/

interface Collection<T> {
    getAll(): FixedArray<T>;
    set(i: int, v: T): void;
}

class StringColl implements Collection<string> {
    private data: FixedArray<string> = ["a"];
    getAll(): FixedArray<string> {
        return this.data;
    }
    set(i: int, v: string) {
        this.data[i] = v;
    }
}

interface Slot<T> {
    get(): FixedArray<T>;
    put(v: T): void;
}

class IntSlot implements Slot<int> {
    data: FixedArray<int> = [0];
    get(): FixedArray<int> {
        return this.data;
    }
    put(v: int) {
        this.data[0] = v;
    }
}

interface Producer<T> {
    produce(): FixedArray<T>;
}

class SP implements Producer<string> {
    produce(): FixedArray<string> {
        return ["x", "y"];
    }
}


function main(): int {

    let testSuite = new arktest.ArkTestsuite("FixedArray_with_generics_interface");

    let exceptionCheckCast = (e: Error) => {
        return ( (e instanceof ClassCastError) && (e.message.includes('cannot be cast to')) );
    }

    let exceptionCheckStore = (e: Error) => {
        return ( (e instanceof ArrayStoreError) && (e.message.includes('cannot be stored in an array of type')) );
    }

    //#1 correct type through interface
    testSuite.addTest("interface_container_positive", () => {
        let c: Collection<string> = new StringColl();
        c.set(0, "b");
        arktest.assertEQ(c.getAll()[0], "b");
    });

    //#2 wrong type through interface bridge
    testSuite.addTest("interface_container_negative", () => {
        arktest.expectThrow(
            () => {
                let s: Slot<Object> = new IntSlot() as Slot<Object>;
                s.put("wrong");
            },
            exceptionCheckCast
        );
    });

    //#3 Covariant return: FixedArray as FixedArray
    testSuite.addTest("interface_covariant_return_positive", () => {
        let p: Producer<Object> = new SP() as Producer<Object>;
        let arr: FixedArray<Object> = p.produce();
        arktest.assertTrue(arr[0] instanceof string);
    });

    //#4 Write wrong type to covariantly-returned array
    testSuite.addTest("interface_container_covariant_negative", () => {
        arktest.expectThrow(
            () => {
                let p: Producer<Object> = new SP() as Producer<Object>;
                let arr: FixedArray<Object> = p.produce();
                arr[0] = 42
            },
            exceptionCheckStore
        );
    });

    return testSuite.run();
}
