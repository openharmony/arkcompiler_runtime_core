/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
 desc: 17.3 Indexable Types
 tags:
 ---*/

class Foo {
    public i: int;
    constructor (i : int) {
        this.i = i;
    }
}


class Bar {
    private data : Foo
    $_get (index: int) : Foo  {return data}
    $_set (index: int, value: Foo) {data = value}

    constructor(data : Foo) {
        this.data = data;
    }
}

function main() {
    let x : Bar = new Bar(new Foo(1));
    assertEQ(x[0].i, 1)
    let y : Foo = new Foo(100);
    x[0] = y;
    assertEQ(x[0].i, 100)
    return 1;
}
