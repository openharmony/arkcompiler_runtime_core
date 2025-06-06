/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Interface level scope, public access
---*/

let x = -10

interface I {
    // this method is overridden by class instance method
    foo(): int {
        return x + 1;
    }

    bar(): int {
        return this.foo() + 2;
    }
}

class C implements I {
    public fld: int;
    // overrides default interface method
    public foo(): int {
        return 100 + this.fld;
    }
}

function main(): void {
    let c: C = new C()

    assertEQ( c.foo(), 100 )
    c.fld = 2
    assertEQ( c.foo(), 102 )

    assertEQ( c.bar(), 104 )
    c.fld = 4
    assertEQ( c.bar(), 106 )

    // module level scope is not affected
    assertEQ( x, -10 )
}
