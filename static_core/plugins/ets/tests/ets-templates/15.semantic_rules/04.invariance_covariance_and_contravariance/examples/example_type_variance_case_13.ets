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
desc: Ensuring that method_one in subclasses keeps parameter and return types unchanged.
tags: [compile-only, negative]
---*/

class Base {
  foo(p: Derived): Derived {
    return p;
  }
}

class Derived extends Base {
  override foo(p: Derived): Base {
    return p;
  }
}

function main(): void {
  try {
    let base = new Base();
    let derived = new Derived();

    derived.foo(derived);
  } catch (error) {
  }
}