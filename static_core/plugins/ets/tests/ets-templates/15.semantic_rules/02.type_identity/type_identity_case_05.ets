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
desc: Ensuring type identity in conditional types.
---*/

function main(): void {
  let value1: number | string = 42;
  let value2: number | string = "Hello";

  let result1 = typeof value1 === "number";
  let result2 = typeof value2 === "number";

  assertEQ(result1, true, "Expected result1 to be true");
  assertEQ(result2, false, "Expected result2 to be false");
}