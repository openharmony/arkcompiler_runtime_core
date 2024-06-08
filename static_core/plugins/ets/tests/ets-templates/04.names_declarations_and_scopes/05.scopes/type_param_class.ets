/*---
Copyright (c) 2021-2024 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
  The scope of a type parameter name in a class or interface declaration is
  that entire declaration, excluding static member declarations.
tags:
- compile-only
- negative
---*/

let T: Number = 1.0

class C<T extends Number> {
    fld1: T;
    fld2: T;

    public foo(p: T): T {
        this.fld1 = p;
        this.fld2 = this.fld1 * 2; // CTE, cannot apply arithmetic to type parameters
        return this.fld2;
    }

    public static bar(): Number {
        return T * 2;
    }
}

function main(): void {
    let c: C<Number> = new C<Number>();

    assert c.foo(2.0) == 4.0
    assert C.bar() == 2.0

    // module level scope is not affected
    assert T == 1.0
}
