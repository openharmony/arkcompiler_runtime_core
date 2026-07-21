/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: Override public default method from superinterface with same signature.
assert: >-
      The relationship between the signature of an interface method and the signatures
      of any overridden interface methods is specified in link to class’ requirements
      in overriding and hiding.
---*/

interface A {
  foo(p: int): int {
      return p + 1;
  }
}

interface B extends A {
  foo(p: int): int {
      return p + 10;
  }
}

function main() {
    let a: A = {}
    arktest.assertEQ(a.foo(1), 2)

    let b: B = {}
    arktest.assertEQ(b.foo(1), 11)

    let x: A = ((): A => b)()
    arktest.assertEQ(x.foo(1), 11)
}
