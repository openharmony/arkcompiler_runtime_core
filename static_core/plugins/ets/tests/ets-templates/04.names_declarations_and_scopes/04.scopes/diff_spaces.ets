/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: A name must be unique in its block space
---*/

function foo(): int {
    let xyz: int;
    return xyz;
}

class C {
    public xyz: int = 1;
}

class D {
    public xyz(): int {
        return 2;
    }
}

enum Enum { abc, def, ghk, xyz }

let xyz: int = 4;

function main(): int {
    if (0 == foo() &&
        1 == new C().xyz &&
        2 == new D().xyz() &&
        3 == Enum.xyz.valueOf() &&
        4 == xyz
    ) {
        let xyz = 0;
        return xyz;
    }
    return 1;
}
