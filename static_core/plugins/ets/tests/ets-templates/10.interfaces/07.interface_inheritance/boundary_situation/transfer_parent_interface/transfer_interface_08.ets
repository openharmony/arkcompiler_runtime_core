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
desc: Null Interface and Function Parameter Constraints.
---*/

interface ParentInterface {
  data: string;
}

interface EmptyInterface {}

interface CombinedInterface extends ParentInterface, EmptyInterface {}

function processData(input: CombinedInterface): string {
  return `Processing data: ${input.data}`;
}

function main(): void {
  const input: CombinedInterface = {
    data: "Hello, World!",
  };

  let resule = processData(input);
  assertEQ(resule, "Processing data: Hello, World!");
}