/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
desc: NullPointerError is thrown by an array access expression
  if the array reference expression has the null value.
---*/

function main(): int {
  let a = 1;
  let b = 1;
  let x: String[]|null = ["A", "B", "C"];

  try {
    let v: String = "" + (x = null) + (a = 2) + x[1] + (b = 2);
    return 1;
  } catch (e: NullPointerError) {
    if (a != 2 || b != 1) return 1;
    return 0;
  } catch {
    return 1;
  }
  return 1;
}
