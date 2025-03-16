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
    static sum(a0: long, a1: long): long {
        return a0 + a1 as long;
    }
}; 

namespace na {
class A {
    static funcA(a0: long, a1: long): long {
        return a0 + a1 as long;
    }
    static funcA(a0: int, a1: int): int {
        return a1 - a0 as int;
    }
};
}

namespace nb {
namespace nc {
class A {
    static funcA(a0: long, a1: long): long {
        return a0 + a1 as long;
    }
};
}
}

namespace nd {
class A {
    static funcA(a0: long, a1: long): long {
        return a0 + a1 as long;
    }
};
class B extends A{
    static funcA(a0: long, a1: long): long {
        return a1 - a0 as long;
    }
};
}