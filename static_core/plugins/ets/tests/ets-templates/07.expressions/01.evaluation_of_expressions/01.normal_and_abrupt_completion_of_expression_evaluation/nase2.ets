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
desc: NegativeArraySizeError is thrown by an array creation expression
  if any dimension expression has a value that is less than zero.
---*/

function main(): int {
  let a: int = 1;
  let b: int = 1;
  let sz: int = 0x0fffffff;

  try {
    let v: int = 42 + (a = 2) + new byte[sz].length + (b = 2);
    if (a != 2 || b != 2) return 1;
    return 0;
  } catch (e: NegativeArraySizeError) {
    return 1;
  } catch (e) {
    return 1;
  }
}
