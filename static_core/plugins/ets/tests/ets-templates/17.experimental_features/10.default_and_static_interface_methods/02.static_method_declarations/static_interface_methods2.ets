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
desc: Static interface method calls refer to no particular object.
---*/

interface I {
    static inc(p: number): number {
        return p + 1
    }
}

interface J extends I {
    static inc(p: number): number {
        return p + 2
    }
}

class A implements I {
    static inc(p: number): number {
        return p + 3
    }
}

class B implements J {
    static inc(p: number): number {
        return p + 4
    }
}

class C extends A {}

class D implements I {}

class F implements J {}

function main(): void {
    assert A.inc(10) == 13
    assert B.inc(10) == 14
    assert C.inc(10) == 13
    assert D.inc(10) == 11
    assert F.inc(10) == 12

    assert I.inc(20) == 21
    assert J.inc(20) == 22
}
