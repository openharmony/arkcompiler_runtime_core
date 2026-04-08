/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: A method defined in a superclass can be used as one of the overloaded methods 
      in an explicit overload of the same-name subclass.
---*/

class C {
    foo(n: number): number {
        return n
    }
}

class D extends C {
    foo(n: number): number {
        return n + 1
    }
    fooString(s: string): string {
        return s;
    }
    overload foo { foo, fooString }
}

function main() {
    let d = new D()
    let c: C = d
    arktest.assertEQ(d.foo(1), 2, "foo from D is called")
    arktest.assertEQ(d.foo("aa"), "aa", "fooString from D is called")
    arktest.assertEQ(c.foo(1), 2, "foo from D is called")
}
