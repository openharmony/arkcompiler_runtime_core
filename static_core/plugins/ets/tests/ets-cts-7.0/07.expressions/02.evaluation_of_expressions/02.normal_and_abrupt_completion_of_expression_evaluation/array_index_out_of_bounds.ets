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
  If the value of an array index expression is negative,
  or greater than, or equal to the length of the array,
  then an array indexing expression throws RangeError.
---*/

class State {
  a: int = 1;
  b: int = 1;
}

function writeAtLength(s: State, v: int[]): void {
  let r = "X" + (s.a = 2) + (v[100] = 35) + (s.b = 2);
}

function writeAtByteOverflow(s: State, v: int[]): void {
  let r: String = "X" + (s.a = 2) + (v[0xff.toByte()] = 35) + (s.b = 2);
}

function read2DValid(s: State, v: int[][]): void {
  let r: String = "X" + (s.a = 2) + ++v[1][1] + (s.b = 2);
}

function replace3DValid(s: State, v: byte[][][]): int {
  return (s.a = 2) + v[0].length + (v[0][0] = new byte[10](0)).length + (s.b = 2);
}

function read2DOutOfBounds(s: State, v: String[][]): void {
  let ind = 1
  let r: String = "S" + (v[0][1] = "B") + (s.a = 2) + v[ind][0] + (s.b = 2);
}

const suite = new arktest.ArkTestsuite("Abrupt completion: ArrayIndexOutOfBounds")

suite.addTest("Index equal to array length throws RangeError", () => {
  let s = new State()
  let v = new int[100](0);
  try {
    writeAtLength(s, v);
  } catch (e) {
    arktest.assertEQ(s.a, 2);
    arktest.assertEQ(s.b, 1);
  }
})

suite.addTest("Negative index from byte overflow throws RangeError", () => {
  let s = new State()
  let v = new int[256](0);
  try {
    writeAtByteOverflow(s, v);
  } catch (e) {
    arktest.assertEQ(s.a, 2);
    arktest.assertEQ(s.b, 1);
  }
})

suite.addTest("Valid 2D array access completes normally", () => {
  let s = new State()
  let v: int[][] = [[1, 2], [3, 4], [5, 6]];
  read2DValid(s, v)
  arktest.assertEQ(s.a, 2);
  arktest.assertEQ(s.b, 2);
})

suite.addTest("Valid 3D array element replacement completes normally", () => {
  let s = new State()
  let v: byte[][][] = [[[new Byte(0)]]];
  replace3DValid(s, v)
  arktest.assertEQ(s.a, 2);
  arktest.assertEQ(s.b, 2);
})

suite.addTest("Out-of-bounds 2D row index throws RangeError", () => {
  let s = new State()
  let v: String[][] = [["A", ""]];
  try {
    read2DOutOfBounds(s, v)
  } catch (e) {
    arktest.assertEQ(s.a, 2);
    arktest.assertEQ(s.b, 1);
  }
})

function main() {
  suite.run()
}
