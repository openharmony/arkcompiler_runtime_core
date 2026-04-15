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
desc: >-
    If a field declaration is an implementation of one or more properties
    inherited from superinterfaces (see Interface Inheritance) then the
    types of the field and all properties must be the same.
    Otherwise, a compile-time error occurs.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0136: Method f(f: B3): void in CC2 not overriding any method"
---*/

// Two unrelated interfaces
interface B1 {}
interface B2 {}
// Interface which extends both of them
interface B3 extends B1, B2 {}
// Class which implements B3
class BB3 implements B3 {}

interface II1 { f: B1 }
interface II3 { f: B1 } // The same property 'f' type as in II1

class CC2 implements II1, II3 {
  f: B3 = new BB3
}
