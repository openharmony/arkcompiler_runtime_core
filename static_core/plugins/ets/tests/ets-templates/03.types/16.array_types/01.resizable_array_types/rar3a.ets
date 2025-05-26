/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
    Testing type erasure of Array<A> elements.
    The same Array accepts objects of different types: A and B.
---*/

class A {}
class B {}

let a = new A
let b = new B

let aa: A[] = []

function main() {
    let obj: Object = aa as Object
    let ab = obj as Array<B>

    ab.push(b)
    aa.push(a)
    ab.push(b)
    aa.push(a)

    assertEQ(aa.length, 4)

    assertEQ(aa.pop(), a)
    assertEQ(aa.length, 3)

    assertEQ(ab.pop(), b)
    assertEQ(aa.length, 2)

    assertEQ(aa.pop(), a)
    assertEQ(aa.length, 1)

    assertEQ(ab.pop(), b)
    assertEQ(aa.length, 0)
  }
