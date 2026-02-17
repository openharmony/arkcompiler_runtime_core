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
    Default method dispatch through a two-level interface hierarchy.
    ParentInterface defines a default method returning 1. TestedInterface
    extends ParentInterface and overrides the method returning 2. The class
    overrides the method returning a distinct value. Dispatch through a cast
    to the parent interface still resolves to the class's own implementation.
assert: >-
    An interface inherits from its direct superinterfaces all methods
    of the superinterfaces. If a class overrides a default method inherited
    through an interface chain, the class's implementation is used for all
    invocations regardless of the reference type used.
---*/

interface ParentInterface {
    testedMethod(): int {
        return 1;
    }
}

interface TestedInterface extends ParentInterface {
    testedMethod(): int {
        return 2;
    }
}

class Impl implements TestedInterface {
    testedMethod(): int {
        return 42;
    }
}

function main(): void {
    let test = new Impl();
    arktest.assertEQ(test.testedMethod(), 42);
    arktest.assertEQ((test as ParentInterface).testedMethod(), 42);
    arktest.assertEQ((test as TestedInterface).testedMethod(), 42);
}
