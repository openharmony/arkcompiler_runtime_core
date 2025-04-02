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

export class ATest {};
class BTest {};
namespace ops {
    class C {} 
};
abstract class Person {
    public abstract addMethod(a0: int, a1: int): int;
};

class Student extends Person {
    static b: int = 1;
    static readonly c: int = 0;
    public override addMethod(a0: int, a1: int): int {
        return a0 + a1;
    }
};

final class Child extends Person {
    public override addMethod(a0: int, a1: int): int {
        return a0 + a1;
    }
};

interface AA {
    foo(x: double): double;
};

interface AAB {
    eat(x: int): int;
};
export class BB implements AA, AAB {
    foo(x: double): double { return x; }
    eat(x: int): int { return x; }
};

class A {}
class B {}
class C {}
class D {}
class E {}
class F {}
class G {}
class H {}
class I {}
class J {}

namespace aaa_test {
    class BBBTest {}
}

class AT {}
class BT extends AT {}
class CT extends BT {}

interface Container<T> {
  add(item: T): void;
}

class ContainerTest<T> implements Container<T>{
    items: (T | undefined)[];
    index: int = 0;
    length: int = 0;
    constructor(length:int) {
        this.items = new (T | undefined)[length];
        this.length = length;
    }
    override add(item: T): void {
        if (this.index >= this.length) {
            return;
        }
        this.items[this.index++] = item;
    }
}
    
