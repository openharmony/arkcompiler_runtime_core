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
  NegativeArraySizeError is thrown by an array creation expression
  if any dimension expression has a value that is less than zero.
---*/

class State {
  a: int = 1;
  b: int = 1;
}

function allocLengthString(s: State, sz: int): void {
  let v: String = "" + (s.a = 2) + new byte[sz](0).length + (s.b = 2);
}

function allocPositive(s: State, sz: int): int {
  return 48 + (s.a = 2) + new byte[sz](0).length + (s.b = 2);
}

function allocFilledString(s: State, sz: int): void {
  let v: String = "" + (s.a = 2) + new byte[sz](1) + (s.b = 2);
}

const suite = new arktest.ArkTestsuite("Abrupt completion: NegativeArraySizeError")
const expectedException = (e: Error): boolean => { return (e instanceof NegativeArraySizeError) }

suite.addTest("Negative size -1 throws NegativeArraySizeError", () => {
  let s = new State()
  arktest.expectThrow(() => { allocLengthString(s, -1); }, expectedException)
  arktest.assertEQ(s.a != 2 || s.b != 1, false)
})

suite.addTest("Positive size completes normally", () => {
  let s = new State()
  allocPositive(s, 10)
  arktest.assertEQ(s.a, 2);
  arktest.assertEQ(s.b, 2);
})

suite.addTest("INT_MIN size throws NegativeArraySizeError", () => {
  let s = new State()
  arktest.expectThrow(() => { allocFilledString(s, -0x80000000); }, expectedException)
  arktest.assertEQ(s.a, 2, "a value not changed")
  arktest.assertEQ(s.b, 1, "b value changed")
})

function main() {
  suite.run()
}
