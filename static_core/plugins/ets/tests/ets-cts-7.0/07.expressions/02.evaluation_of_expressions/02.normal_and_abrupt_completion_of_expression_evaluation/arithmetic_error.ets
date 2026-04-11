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
  ArithmeticError is thrown by an integer division or integer
  remainder operator if the right-hand operand expression has the zero value.
---*/

class State {
  a: int = 1;
  b: int = 1;
  i: int = 1;
  j: int = 1;
}

class LongState {
  a: int = 1;
  b: int = 1;
  i: long = 0x00000000ffffffff;
  j: long = 0x0fffffff00000000;
}

function divDecJ(s: State): void {
  let v: String = "X" + (s.a = 2) + s.i / --s.j * (s.b = 2);
}

function remDecJ(s: State): void {
  let v: String = "X" + (s.a = 2) + s.i % --s.j * (s.b = 2);
}

function divNarrowing(s: LongState): void {
  let v: String = "X" + (s.a = 2) + (s.i.toInt()) / (s.j.toInt()) / (s.b = 2);
}

function remPostInc(s: State): void {
  let v: String = "X" + s.a++ % (s.a++ >> 1) % (s.b = 2);
}

function remNormal(s: State): void {
  let v: String = "X" + (s.a = 2) + s.i % s.j-- + (s.b = 2);
}

function divUnsignedShift(s: State): void {
  let v: String = "X" + (s.a = 2) + --s.i / (s.j >>> 1) + (s.b = 2);
}

function orShortCircuit(s: State): void {
  let v: boolean = (++s.a > 0) || (s.i / s.j) > 0;
}

function andEvalRhs(s: State): void {
  let v: boolean = (++s.a > 0) && (s.b++ * s.b++ / s.j) > 0;
}

const suite = new arktest.ArkTestsuite("Abrupt completion: ArithmeticError")
const expectedException = (e: Error): boolean => { return (e instanceof ArithmeticError) }

suite.addTest("Division by zero aborts and stops evaluating rhs", () => {
  let s = new State()
  arktest.expectThrow(() => { divDecJ(s); }, expectedException)
  arktest.assertEQ(s.a != 2 || s.b != 1, false)
})

suite.addTest("Remainder by zero aborts and stops evaluating rhs", () => {
  let s = new State()
  arktest.expectThrow(() => { remDecJ(s); }, expectedException)
  arktest.assertEQ(s.a != 2 || s.b != 1, false)
})

suite.addTest("Division by zero from long-to-int narrowing", () => {
  let s = new LongState()
  arktest.expectThrow(() => { divNarrowing(s); }, expectedException)
  arktest.assertEQ(s.a != 2 || s.b != 1, false)
})

suite.addTest("Remainder by zero with post-increment operands", () => {
  let s = new State()
  s.a = 0;
  arktest.expectThrow(() => { remPostInc(s); }, expectedException)
  arktest.assertEQ(s.a != 2 || s.b != 1, false)
})

suite.addTest("Normal completion when divisor is non-zero", () => {
  let s = new State()
  remNormal(s)
  arktest.assertEQ(s.a, 2)
  arktest.assertEQ(s.b, 2)
})

suite.addTest("Division by zero with unsigned right shift producing zero", () => {
  let s = new State()
  arktest.expectThrow(() => { divUnsignedShift(s); }, expectedException)
  arktest.assertEQ(s.a != 2 || s.b != 1, false)
})

suite.addTest("Short-circuit OR skips division by zero on rhs", () => {
  let s = new State()
  s.j = 0;
  orShortCircuit(s)
  arktest.assertTrue(s.a == 2 || s.b == 1)
})

suite.addTest("Short-circuit AND evaluates rhs causing ArithmeticError", () => {
  let s = new State()
  s.j = 0;
  arktest.expectThrow(() => { andEvalRhs(s); }, expectedException)
  arktest.assertEQ(s.a != 2 || s.b != 3, false)
})

function main() {
  suite.run()
}
