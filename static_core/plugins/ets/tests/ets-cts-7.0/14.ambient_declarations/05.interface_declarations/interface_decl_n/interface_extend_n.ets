/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Ambient Interface implementations
tags: [negative, compile-only]
expected_out: >-
    Semantic error ESE985118: Class member a with type String|undefined in Extended cannot implement a with type String property from interface A
    because the implementing return type is not compatible with the interface return type.
---*/

import { A } from './interface_declarations2.d'

interface Extended extends A {
  a?: string; //Error string|undefined to type string
}

class B implements A {
    a: string = 'b'
}

function main() {
    const c: B  = new B()
}