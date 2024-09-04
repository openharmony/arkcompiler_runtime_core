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
desc: 09.9 Constructor Declaration
tags: []
---*/

class Y {
    dataX1 : int;
    dataX2 : int;

    constructor(p1: int = 0, p2: int = 1) {
        this.dataX1 = p1;
        this.dataX2 = p2;
    }
}

class X extends Y {
    dataY1 : int;
    constructor (p1 : int) {
        super();
        this.dataY1 = p1;
    }
}

function main() : int {
    let x : X = new X(1000);
    assert x.dataY1 == 1000;
    assert x.dataX1 == 0;
    assert x.dataX2 == 1;
    return 0;
}
