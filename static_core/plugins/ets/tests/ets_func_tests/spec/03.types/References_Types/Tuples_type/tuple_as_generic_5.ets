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
name: tuple_as_generic_5.ets
tags: []
---*/

class TuplePair<X,Y> {
    one: X;
    two: Y;

    constructor(p0: X, p1:Y) {
        this.one = p0;
        this.two = p1;
    }

    public provide(): [X, Y] {
        return [this.one, this.two];
    }
}

function main(): void {
    let tuple: TuplePair<String, Double> = new TuplePair<String, Double>("Hello", 12345.0);

    let result = tuple.provide();

    assertEQ( result[0], "Hello")
    assertEQ( result[1], 12345.0)
}
