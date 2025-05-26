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
desc: Verifying implicit conversion from number to string.
tags: [compile-only, negative]
---*/

interface I {
  i: number;
}

class A {
  i: string;
}

function main(): void {
  const val1: number = 123;
  const val2: string = val1.toString();

  const result = checkTypeIdentity(val1, val2);

  assertEQ(result, false);
}