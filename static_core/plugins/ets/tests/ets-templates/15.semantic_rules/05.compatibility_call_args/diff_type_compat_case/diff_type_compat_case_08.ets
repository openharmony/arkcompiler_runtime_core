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
desc: Ensuring type identity in type guards.
tags: [compile-only, negative]
---*/

interface I {
  i: string | number;
}

class A {
  i: string & number;
}

function main(): void {
  const val1: string | number = "test";
  const val2: string & number = {} as never;

  const result = checkTypeIdentity(val1, val2);

  assertEQ(result, false);
}