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
desc: Verifying compatibility in multiple inheritance.
tags: [compile-only, negative]
---*/

class Parent1 {}
class Parent2 {}
class Child extends Parent1, Parent2 {}\

interface I {
  i: Parent1;
}

class A {
  i: Child;
}

function main(): void {
  const val1: Child = new Child();
  const val2: Parent1 = new Parent1();

  const result1 = checkTypeIdentity(val1, val2);
  const result2 = checkTypeIdentity(val2, val1);

  assertEQ(result1, true);
  assertEQ(result2, false);
}