/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: The function name is the same, but the parameter type is different.
---*/

function bar(p: string) {
  return p;
}
function bar(p: number) {
  return p;
}

function main(): void {
  let a: string = bar("abc");
  assertEQ(a, 'abc');

  let b: number = bar(123);
  assertEQ(b, 123);
}