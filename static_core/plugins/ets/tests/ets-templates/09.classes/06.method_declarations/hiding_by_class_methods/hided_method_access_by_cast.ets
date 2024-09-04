/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
desc: Access to hided method by cast.
assert: It is a compile-time error if a static method hides an instance method.
---*/

class A {
 public m(): int {
    return 1;
  }
}

class C extends A {
  public m(): int {
    return 5;
  }
}

function main(): void {
  let instance: A = new C();

  assert instance.m() == 1;
}
