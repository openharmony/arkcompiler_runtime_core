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
desc: An empty interface inherits an interface that contains a conditional type.
---*/

interface ParentInterface<T> {
  value: T;
}

interface EmptyInterface<T> extends ParentInterface<T> {}

function isString(value: string | number | boolean): boolean {
  return typeof value === "string";
}

function main(): void {
  const obj1: EmptyInterface<string> = {
    value: "string"
  };

  const obj2: EmptyInterface<number> = {
    value: 123
  };

  assertEQ(typeof obj1.value, "string");
  assertEQ(typeof obj2.value, "number");
}