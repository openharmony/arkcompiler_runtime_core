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
desc: >-
  Expressions may contain assignments, increment operators, decrement operators,
  method calls and function calls; as a result, an evaluation of an expression
  can produce side effects.
---*/

let sg: String = "a";
let ig: int = 39;

function foo(p: int, s: String): int {
    sg += s;
    ig = -1;
    return p - s.length.toInt();
}

class A {
  meth(p: int, s: String): int {
    sg += s;
    ig = 1;
    return p - s.length.toInt();
  }
}

const suite = new arktest.ArkTestsuite("Expression evaluation side effects")

suite.addTest("Assignment side effect", () => {
  sg = "a";
  ig = 39;
  let i: int = (sg += "b").length.toInt() + (ig = 43);
  arktest.assertEQ(i, 45)
  arktest.assertEQ(sg, "ab")
  arktest.assertEQ(ig, 43)
})

suite.addTest("Increment side effect", () => {
  ig = 43;
  let k: int = ig++ + ++ig;
  arktest.assertEQ(k, 88)
  arktest.assertEQ(ig, 45)
})

suite.addTest("Decrement side effect", () => {
  ig = 45;
  let j: int = ig-- + --ig;
  arktest.assertEQ(j, 88)
  arktest.assertEQ(ig, 43)
})

suite.addTest("Function call side effect", () => {
  sg = "ab";
  ig = 43;
  let m: int = foo(88, sg);
  arktest.assertEQ(m, 86)
  arktest.assertEQ(ig, -1)
  arktest.assertEQ(sg, "abab")
})

suite.addTest("Method call side effect", () => {
  sg = "abab";
  ig = -1;
  let n: int = new A().meth(86, sg);
  arktest.assertEQ(n, 82)
  arktest.assertEQ(ig, 1)
  arktest.assertEQ(sg, "abababab")
})

function main() {
  suite.run()
}
