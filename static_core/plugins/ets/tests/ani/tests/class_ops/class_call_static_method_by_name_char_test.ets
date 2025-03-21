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
        let c: Char = new Char('0');
        if (c0 > c1) {
            c = c0 - c1 as Char;
        } else {
            c = c1 - c0 as Char;
        }
        return c as char;
    }
};

namespace na {
class A {
    static funcA(c0: char, c1: char): char {
        let c: Char = new Char('0');
        if (c0 > c1) {
            c = c0 - c1 as Char;
        } else {
            c = c1 - c0 as Char;
        }
        return c as char;
    }
    static funcA(c0: int, c1: int): int {
        return c1 - c0 as int;
    }
};
}

namespace nb {
namespace nc {
class A {
    static funcA(c0: char, c1: char): char {
        let c: Char = new Char('0');
        if (c0 > c1) {
            c = c0 - c1 as Char;
        } else {
            c = c1 - c0 as Char;
        }
        return c as char;
    }
};
}
}

namespace nd {
class A {
    static funcA(c0: char, c1: char): char {
        let c: Char = new Char('0');
        if (c0 > c1) {
            c = c0 - c1 as Char;
        } else {
            c = c1 - c0 as Char;
        }
        return c as char;
    }
};
class B extends A{
    static funcA(c0: char, c1: char): char {
        let c: Char = new Char('0');
        c = c0 + c1 as Char;
        return c as char;
    }
};
}