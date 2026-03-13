/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
file_name: ani_test_class_ops_class_call_static_method_by_name_boolean
---*/

class Operations {
    static or(a0: boolean, a1: boolean): boolean {
        return a0 || a1;
    }
}

namespace na {
class A {
    static funcA(a0: boolean, a1: boolean): boolean {
        return a0 || a1;
    }
    static funcA(a0: int, a1: int): int {
        return a0 + a1 as int;
    }
}
}

namespace nb {
namespace nc {
class A {
    static funcA(a0: boolean, a1: boolean): boolean {
        return a0 || a1;
    }
}
}
}

namespace nd {
class A {
    static funcA(a0: boolean, a1: boolean): boolean {
        return a0 || a1;
    }
}

class B extends A{
    static funcA(a0: boolean, a1: boolean): boolean {
        return a0 && a1;
    }
}
}

class A {
    static funcA(a0: boolean, a1: boolean): boolean {
        return a0 || a1;
    }

    static funcA(a0: int, a1: int): int {
        return a0 + a1 as int;
    }

    static funcB(a0: boolean, a1: boolean): boolean {
        return A.funcA(a0, a1);
    }
}

class B extends A{
    static funcB(a0: boolean, a1: boolean): boolean {
        return a0 && a1;
    }
}

class C extends A{
}

class D extends A{
    static funcA(a0: boolean, a1: boolean): boolean {
        return a0 && a1;
    }
}

class E extends A{
    static funcA(a0: boolean, a1: boolean): boolean {
        return A.funcA(a0, a1);
    }
}

class F {
    static increment(a0: boolean, a1: boolean): void {
        F.count &= a0;
        F.count &= a1;
    }

    static getCount(): boolean {
        return F.count;
    }

    static count: boolean = false;
}

class G {
    public static publicMethod(a0: boolean, a1: boolean): boolean {
        return a0 || a1;
    }

    private static privateMethod(a0: boolean, a1: boolean): boolean {
        return a0 && a1;
    }

    static callPrivateMethod(a0: boolean, a1: boolean): boolean {
        return G.privateMethod(a0, a1);
    }
}

class CheckWrongSignature {
    static method(a: String): boolean {
        return true;
    }
}

class Parent {
    static parentStaticMethod(): boolean {
        return true;
    }
}
class Child extends Parent {
    static childStaticMethod(): boolean {
        return true;
    }
}
final class Grandchild extends Child {
    static grandchildStaticMethod(): boolean {
        return true;
    }
}
