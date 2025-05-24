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

class Operations {
    static sub(c0: char, c1: char): char {
        let c: Char = new Char(c'0');
        if (c0 > c1) {
            c = c0 - c1 as Char;
        } else {
            c = c1 - c0 as Char;
        }
        return c as char;
    }
}

class A {
    static funcA(a0: char, a1: char): char {
        let c: Char = new Char(c'0');
        if (a0 > a1) {
            c = a0 - a1 as Char;
        } else {
            c = a1 - a0 as Char;
        }
        return c as char;
    }

    static funcA(a0: int, a1: int): int {
        return a0 + a1 as int;
    }

    static funcB(a0: char, a1: char): char {
        return A.funcA(a0, a1);
    }
}

class B extends A{
    static funcB(a0: char, a1: char): char {
        let c: Char = new Char(c'0');
        c = a0 + a1 as Char;
        return c as char;
    }
}

class C extends A{
}

class D extends A{
    static funcA(a0: char, a1: char): char {
        let c: Char = new Char(c'0');
        c = a0 + a1 as char;
        return c as char;
    }
}

class E extends A{
    static funcA(a0: char, a1: char): char {
        return A.funcA(a0, a1);
    }
}

class F {
    static increment(a0: char, a1: char): void {
        F.count = F.count + a0 as char;
        F.count = F.count + a1 as char;
    }

    static getCount(): char {
        return F.count;
    }

    static count: char = c'\0';
}

class G {
    public static publicMethod(a0: char, a1: char): char {
        let c: Char = new Char(c'0');
        c = a0 + a1 as char;
        return c as char;
    }

    private static privateMethod(a0: char, a1: char): char {
        let c: Char = new Char(c'0');
        if (a0 > a1) {
            c = a0 - a1 as char;
        } else {
            c = a1 - a0 as char;
        }
        return c as char;
    }

    static callPrivateMethod(a0: char, a1: char): char {
        return G.privateMethod(a0, a1);
    }
}
