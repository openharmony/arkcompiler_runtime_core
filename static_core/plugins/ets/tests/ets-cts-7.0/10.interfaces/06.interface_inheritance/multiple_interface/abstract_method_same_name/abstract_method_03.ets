/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Method signature of the same name is compatible.
---*/

let num: number = 0
let str: string = ""

interface Parent1 {
  greet(message: string): void;
}

interface Parent2 {
  greet(message: number): void;
}

interface Child extends Parent1, Parent2 {
  greet(message: string | number): void;
}
class ChildClass implements Child {
  greet(message: string | number): void {
    if(message instanceof string) {
      str = message
    } else if ( message instanceof number ) {
      num = message
    }
  }
}

function main() {
  num = 0
  str = ""
  let i: Child = new ChildClass()
  i.greet("some str")
  i.greet(5.6)
  arktest.assertEQ(str, "some str")
  arktest.assertEQ(num, 5.6)
}
