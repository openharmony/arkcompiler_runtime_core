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
desc: Ensuring type identity in union type usage.
---*/

type MyUnionType = string | number;

function processValue(value: MyUnionType): MyUnionType {
  return value;
}

function main(): void {
  let result1 = processValue("Hello");
  let result2 = processValue(42);

  assertEQ(result1, "Hello");
  assertEQ(result2, 42);
}