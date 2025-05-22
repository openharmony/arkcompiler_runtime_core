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
desc: Bitwise complement operator
tags: [negative, compile-only]
---*/

function main(): void {
  let a: double = 9223372036854775808;
  assertEQ(~a, -9223372036854775808);

  let b: double = 9223372036854775809;
  assertEQ(~b, -9223372036854775808);
git}
