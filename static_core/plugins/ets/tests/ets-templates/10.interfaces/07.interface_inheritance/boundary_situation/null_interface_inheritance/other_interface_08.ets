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
desc: The null interface inherits an interface typescript that contains union types.
---*/

interface ParentInterface {
  value: string | number;
}

interface EmptyInterface extends ParentInterface {}

function main(): void {
  const obj1: EmptyInterface = {
    value: "Hello"
  };

  const obj2: EmptyInterface = {
    value: 42
  };

  assertEQ(obj1.value, "Hello");
  assertEQ(obj2.value, 42);
}