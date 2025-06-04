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
desc: Null interfaces and union types.
---*/

interface ParentInterface {
  key: string;
}

interface EmptyInterface extends ParentInterface {}

function processCombinedType(item: EmptyInterface): string {
  return `Key: ${item.key}`;
}

function main(): void {
  const combinedItem: EmptyInterface = {
    key: "exampleKey"
  };
  let resule = processCombinedType(combinedItem);

  assertEQ(resule, "Key: exampleKey");
}