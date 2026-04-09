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
  ClassCastError is thrown by a cast expression
  if at run time a cast cannot be performed.
---*/

class State {
  a: int = 1;
  b: int = 1;
}

function castObjectToLong(s: State, o: Object): void {
  let v: Long = (s.a = 2) + new Long((o as Long)) + (s.b = 2);
}

function nullSafeAccess(s: State, t: Long | null): long {
  return (s.a = 2) + (t === null ? 0 : t) + (s.b = 2);
}

const suite = new arktest.ArkTestsuite("Abrupt completion: ClassCastError")
const expectedException = (e: Error): boolean => { return (e instanceof ClassCastError) }

suite.addTest("Invalid cast from Object to Long throws ClassCastError", () => {
  let s = new State()
  let o = new Object();
  arktest.expectThrow(() => { castObjectToLong(s, o); }, expectedException)
  arktest.assertEQ(s.a != 2 || s.b != 1, false)
})

suite.addTest("Null-safe access completes normally without cast", () => {
  let s = new State()
  let t: Long | null = (): Long | null => { return null; }();
  nullSafeAccess(s, t)
  arktest.assertEQ(s.a, 2);
  arktest.assertEQ(s.b, 2);
})

function main() {
  suite.run()
}
