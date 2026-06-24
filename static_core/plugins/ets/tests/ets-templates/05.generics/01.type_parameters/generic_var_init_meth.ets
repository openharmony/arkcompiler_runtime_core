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
    No type parameter has a default value, and initialization is mandatory for variables
    and fields of a type parameter (see Field Initialization)
---*/

class G<T> {
  // note similar test for class field is in 9.6.4
  foo(v: T) {
    let t: T
    t = v
    return t
  }
}

function main(): void {
  let g = new G<string>
  arktest.assertEQ(g.foo("string"), "string", "expect string")
}
