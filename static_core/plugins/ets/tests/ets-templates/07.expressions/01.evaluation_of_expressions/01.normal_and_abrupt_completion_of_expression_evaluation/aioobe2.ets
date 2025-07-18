/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: If the value of an array index expression is negative,
  or greater than, or equal to the length of the array,
  then an array indexing expression throws RangeError.
tags:
---*/

function main(): int {
  let v = new int[256];
  let a = 1;
  let b = 1;

  try {
    // Negative because (v[0xff as byte] = 42) returns void
    let s: String = "X" + (a = 2) + (v[0xff as byte] = 42) + (b = 2);
    return 1;
  } catch (e) {
    if (a == 2 && b == 1) return 0;
  }
  return 1;
}
