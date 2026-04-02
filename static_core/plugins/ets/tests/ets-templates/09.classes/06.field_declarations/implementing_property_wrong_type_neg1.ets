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
expected_out: >-
  Semantic error ESE985118: Class member f with type B1 in CC1 cannot implement f with type B2 property from interface II2
  because the implementing return type is not compatible with the interface return type.
---*/

// Two unrelated interfaces
interface B1 {}
interface B2 {}
// Interface which extends both of them
interface B3 extends B1, B2 {}
// Class which implements B3
class BB3 implements B3 {}

interface II1 { f: B1 }
interface II2 { f: B2 } // Different property 'f' type as in II1

class CC1 implements II1, II2 {
  f: B1 = new BB3
}
