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
desc: Tests calling subclass rewrite methods through parent class references.
---*/

class A {
  method(): string {
    return "Method from A";
  }
}

class B extends A {
  method(): string {
    return super.method() + " and B";
  }
}

function main(): void {
  let bInstance: A = new B();
  let b = bInstance.method();
  assertEQ(b, "Method from A and B");
}