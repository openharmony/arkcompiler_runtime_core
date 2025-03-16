/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Different Namespaces in common file
tags: [not-a-test]
---*/

package pack1

export namespace A {
    export function foo() {return "A.foo"}
    function bar(): string {return "A.bar"}

    export namespace C {
        export function too() {return "A.C.too"}
    }
}

namespace B {
     export namespace D {
         export function too() {return "B.D.too"}
     }
 }
