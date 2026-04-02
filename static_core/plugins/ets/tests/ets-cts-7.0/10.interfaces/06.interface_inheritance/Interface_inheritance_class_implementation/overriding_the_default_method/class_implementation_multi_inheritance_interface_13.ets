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
    Generic default method dispatch through multiple interface inheritance
    with transitive inheritance. SuperParent1 has a generic default method
    returning 1. SuperParent2 is empty. ParentInterface extends both and
    overrides the method returning 3. ChildInterface extends ParentInterface
    without adding members. The class overrides the method returning a
    distinct value. Dispatch through a cast to any interface that declares
    the method resolves to the class's own implementation.
assert: >-
    An interface inherits from its direct superinterfaces all methods
    of the superinterfaces. If a class overrides a generic default method
    inherited through an interface chain, the class's implementation is
    used for all invocations regardless of the reference type used.
---*/

interface SuperParent1 {
    testedMethod<T>(): int {
        return 1;
    }
}

interface SuperParent2 {}

interface ParentInterface extends SuperParent1, SuperParent2 {
    testedMethod<T>(): int {
        return 3;
    }
}

interface ChildInterface extends ParentInterface {}

class Impl implements ChildInterface {
    testedMethod<T>(): int {
        return 99;
    }
}

function main(): void {
    let test = new Impl();
    arktest.assertEQ(test.testedMethod<Object>(), 99);
    arktest.assertEQ((test as SuperParent1).testedMethod<Object>(), 99);
    arktest.assertEQ((test as ParentInterface).testedMethod<Object>(), 99);
    arktest.assertEQ((test as ChildInterface).testedMethod<Object>(), 99);
    // Dispatch is type-argument-independent — class override wins regardless of T
    arktest.assertEQ(test.testedMethod<Int>(), 99);
    arktest.assertEQ((test as SuperParent1).testedMethod<String>(), 99);
}
