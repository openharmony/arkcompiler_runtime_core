/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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


class X0 {
    static foo() : int {return 0;};
}

class X1  extends X0 {
    foo() : int {return 1;};
}

class X2  extends X1 {
    static foo() : int {return 2;};
}

class X3  extends X2 {
    foo() : int {return 3;};
}


function main() {
    let x0 : X0 = new X0();
    let x1 : X1 = new X1();
    let x2 : X2 = new X2();
    let x3 : X3 = new X3();

    assert X0.foo() == 0
    assert x1.foo() == 1
    assert X2.foo() == 2
    assert x3.foo() == 3
}
