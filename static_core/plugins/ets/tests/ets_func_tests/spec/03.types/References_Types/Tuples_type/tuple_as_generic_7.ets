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
desc: 03.8.10 Tuple Types
name: tuple_as_generic_7.ets
tags: []
---*/

class TupleProvider<X> {
    one: X;

    constructor(value: X) {
        this.one = value;
    }

    public publish(): [X, X] {
        return [this.one, this.one];
    }
}

function main(): void {

    let ts: TupleProvider<String> = new TupleProvider<String>("Hello World");
    let tn: TupleProvider<number> = new TupleProvider<number>(42.0);

    let ress = ts.publish();
    let resn = tn.publish();

    assertEQ( ress[0], "Hello World")
    assertEQ( ress[1], "Hello World")

    assertEQ( resn[0], 42.0)
    assertEQ( resn[1], 42.0)
}
