/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: 9.6 Method_Declaration
tags: []
---*/


class X {
    static foo() : int {return 0;};
}

class Y extends X {
    foo() : int {return 1};
}

function main() {
    let x : X = new X();
    let y : Y = new Y();

    assertEQ( y.foo(), 1)
    assertEQ( Y.foo(), 0)
    assertEQ( X.foo(), 0)
}
