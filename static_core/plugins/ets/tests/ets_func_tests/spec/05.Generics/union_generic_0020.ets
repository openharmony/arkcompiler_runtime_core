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
desc: 05 Generics
tags: []
---*/

class A1<T> {
    private attr: String;
    private attr2: T;

    constructor(p_attr: String, p_attr2: T ) {
        this.attr = p_attr;
        this.attr2 = p_attr2;
    }

    public foo() : String {
        return (this.attr + this.attr2);
    }

}

class A2 {
    private attr: String;
    private attr2: number;

    constructor(p_attr : String, p_attr2: number) {
        this.attr = p_attr;
        this.attr2 = p_attr2;
    }

    public bar() : String {
        return (this.attr + this.attr2)
    }
}

type A12<T> = A1<T> | A2;

function main(): int {
    let var1 : A12<Number>;
    let var2 : A12<Number>;

    var1 = new A1<Number>("Hello", 496)
    var2 = new A2("Bye", 28);

    assertEQ( var1.foo(), "Hello496")
    assertEQ( var2.bar(), "Bye28")
    return 0;
}
