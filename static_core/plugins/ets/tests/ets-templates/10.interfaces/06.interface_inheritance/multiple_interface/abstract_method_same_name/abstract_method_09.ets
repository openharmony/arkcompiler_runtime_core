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
desc: The method with the same name has the same parameter type but different return types.
tags: [compile-only, negative]
---*/

interface Parent1 {
  calculate(value: number): number;
}

interface Parent2 {
  calculate(value: number): string;
}

interface Child extends Parent1, Parent2 {}

class ChildClass implements Child {
  processString(data: string): string {
    return `Processed String: ${data}`;
  }

  processNumber(data: number): number {
    return data * 2;
  }
}