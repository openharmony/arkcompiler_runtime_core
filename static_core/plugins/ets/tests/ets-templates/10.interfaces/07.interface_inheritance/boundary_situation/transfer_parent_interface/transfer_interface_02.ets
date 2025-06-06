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
desc: Null interface as a type constraint.
---*/

interface ParentInterface {
  id: number;
  description: string;
}

interface EmptyInterface {}

interface CombinedInterface extends EmptyInterface, ParentInterface {}

function printInfo<T extends CombinedInterface>(item: T): string {
  return `ID: ${item.id}, Description: ${item.description}`;
}

function main(): void {
  const item: CombinedInterface = {
    id: 1,
    description: "This is an item"
  };

  let result = printInfo(item);
  assertEQ(result, "ID: 1, Description: This is an item");
}