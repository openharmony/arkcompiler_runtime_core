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
  NullPointerError is thrown by an array access expression
  if the array reference expression has the null value.
---*/

class State {
  a: int = 1;
  b: int = 1;
  byt: byte = 0;
}

function makeByteArray(i: int): byte[]|null {
  return i <= 0 ? null : new byte[i](0);
}

function makeByteLiteral(i: byte): byte[]|null {
  return i <= 0 ? null : [i];
}

function makeStringArray(i: int): String[]|null {
  return i <= 0 ? null : ["A", "B", "C"];
}

function readNullByte(s: State): void {
  let v: String = "X" + (s.a = 2) + makeByteArray(-1)![0] + (s.b = 2);
}

function readValidByte(s: State): void {
  let v: String = "X" + (s.a = 2) + (s.byt = makeByteLiteral((48).toByte())![0]) + (s.b = 2);
}

function readNullString(s: State): void {
  let v: String = "" + (s.a = 2) + makeStringArray(-1)![1] + (s.b = 2);
}

const suite = new arktest.ArkTestsuite("Abrupt completion: NullPointerError")
const expectedException = (e: Error): boolean => { return (e instanceof NullPointerError) }

suite.addTest("Null byte array access throws NullPointerError", () => {
  let s = new State()
  arktest.expectThrow(() => { readNullByte(s); }, expectedException)
  arktest.assertFalse(s.a != 2 || s.b != 1)
})

suite.addTest("Non-null array access completes normally", () => {
  let s = new State()
  readValidByte(s)
  arktest.assertEQ(s.a, 2);
  arktest.assertEQ(s.b, 2);
})

suite.addTest("Null String array access throws NullPointerError", () => {
  let s = new State()
  arktest.expectThrow(() => { readNullString(s); }, expectedException)
  arktest.assertFalse(s.a != 2 || s.b != 1)
})

function main() {
  suite.run()
}
