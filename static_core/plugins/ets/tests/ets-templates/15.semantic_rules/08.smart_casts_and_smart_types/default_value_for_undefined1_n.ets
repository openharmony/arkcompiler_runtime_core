/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: >-
    Smart type for default value of class member for type undefined.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0318: Type 'String|undefined' cannot be assigned to type 'undefined'"
---*/

class C {
    o: string | undefined
}

function main() {
  let c = new C
  arktest.assertEQ(c.o, undefined, "default value for type [string | undefined]")
  let x: undefined = c.o
  arktest.assertEQ(x, undefined, "undefined via smart type for type [string | undefined]")
}
