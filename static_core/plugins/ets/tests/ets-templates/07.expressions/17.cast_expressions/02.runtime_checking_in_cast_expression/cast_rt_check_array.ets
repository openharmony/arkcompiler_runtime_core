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
    Runtime checking of cast expressions on array elements.
    An array of interface-typed elements with polymorphic actual types
    is cast element-by-element to another interface or class type.
assert: >-
    The result of an as expression is the result of the evaluated expr
    if the actual type of expr is a subtype of target (see Subtyping).
    Otherwise, ClassCastError is thrown.
---*/

interface SomeInterface {
    value(): int
}

interface AnotherInterface {
    label(): string
}

class SomeClass implements SomeInterface {
    override value(): int { return 1; }
}

class SomeSubclass extends SomeClass implements AnotherInterface {
    override value(): int { return 2; }
    override label(): string { return "sub"; }
}

function main(): void {
    let arr: SomeInterface[] = [new SomeSubclass(), new SomeClass(), new SomeSubclass()];

    // arr[0] is SomeSubclass (implements both) -> AnotherInterface: success
    let a: AnotherInterface = arr[0] as AnotherInterface;
    arktest.assertEQ(a.label(), "sub");

    // arr[1] is SomeClass (only SomeInterface) -> AnotherInterface: fail
    let caught = false;
    try {
        let b: AnotherInterface = arr[1] as AnotherInterface;
    } catch (e) {
        caught = true;
        arktest.assertTrue(e instanceof ClassCastError);
    }
    arktest.assertTrue(caught);

    // arr[2] is SomeSubclass (extends SomeClass) -> cast to SomeClass: success
    let c: SomeClass = arr[2] as SomeClass;
    arktest.assertEQ(c.value(), 2);

    // map-like cast: all SomeInterface elements to SomeClass
    let mapped: SomeClass[] = arr.map((e: SomeInterface): SomeClass => e as SomeClass);
    arktest.assertEQ(mapped.length, 3);
    arktest.assertEQ(mapped[0].value(), 2);
    arktest.assertEQ(mapped[1].value(), 1);
    arktest.assertEQ(mapped[2].value(), 2);
}
