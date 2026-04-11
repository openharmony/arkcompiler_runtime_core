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
  The type of a standalone expression (like just a + b) can be determined entirely
  from the content of the expression; the type of any other expression may be
  influenced by its target type.
---*/

function foo(a: byte): int {
  return 1;
}
function foo(a: short): int {
  return 2;
}
function foo(a: char): int {
  return 3;
}
function foo(a: int): int {
  return 4;
}
function foo(a: long): int {
  return 5;
}
function foo(a: float): int {
  return 6;
}
function foo(a: double): int {
  return 7;
}
function foo(a: boolean): int {
  return 8;
}
function foo(a: string): int {
  return 31;
}
function foo(a: FixedArray<byte>): int {
  return 101;
}
function foo(a: FixedArray<short>): int {
  return 102;
}
function foo(a: FixedArray<char>): int {
  return 103;
}
function foo(a: FixedArray<int>): int {
  return 104;
}
function foo(a: FixedArray<long>): int {
  return 105;
}
function foo(a: FixedArray<float>): int {
  return 106;
}
function foo(a: FixedArray<double>): int {
  return 107;
}
function foo(a: FixedArray<boolean>): int {
  return 108;
}
function foo(a: Object): int {
  return 32;
}

const suite = new arktest.ArkTestsuite("Standalone expression type resolution via overload")

suite.addTest("Int arithmetic yields int", () => {
  let v = 1;
  let w = v * 12;
  arktest.assertEQ(foo(v + w), 4)
})

suite.addTest("Mixed long/int assignment yields long", () => {
  let v: long = 1;
  let w: int = 12;
  arktest.assertEQ(foo(v = w / 2 + (v.toShort()) - (48).toByte()), 5)
})

suite.addTest("Array literal with short element yields FixedArray<short>", () => {
  arktest.assertEQ(foo([(1).toShort(), 2, 3]), 102)
})

suite.addTest("Array literal with Object and string yields Object", () => {
  arktest.assertEQ(foo([new Object(), "abc"]), 32)
})

suite.addTest("Array literal with Int yields FixedArray<int>", () => {
  arktest.assertEQ(foo([new Int(1), 2, 3]), 104)
})

suite.addTest("Double array element access yields double", () => {
  let d: Double[];
  arktest.assertEQ(foo(14 + (d = [1.0, 2.0, 3.0])[1]), 7)
})

suite.addTest("Long array assignment expression yields long", () => {
  let k = new Long[3](0);
  arktest.assertEQ(foo(14 + (k[2] = (2).toLong()) + k[2] * (k[1] = (1).toLong())), 5)
})

function main() {
  suite.run()
}
