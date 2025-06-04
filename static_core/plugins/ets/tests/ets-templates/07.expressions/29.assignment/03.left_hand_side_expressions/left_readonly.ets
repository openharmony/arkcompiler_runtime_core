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
desc: Left-hand expression testing - readonly properties as left-hand side values.
tags: [compile-only, negative]
---*/

class MyClass {
  readonly readOnlyProp: number = 100;
}

function main(): void {
  let obj: MyClass = new MyClass();
  obj.readOnlyProp = 200;
  assertEQ(obj.readOnlyProp, 200);
}