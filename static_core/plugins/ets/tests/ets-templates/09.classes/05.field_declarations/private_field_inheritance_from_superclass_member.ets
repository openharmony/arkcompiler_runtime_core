/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Inheritance private field from of a superclass, if both classes are members of same class.
assert: Nevertheless, a private field is never inherited by a superclass.
tags: [negative, compile-only]
---*/

class A {
  inner class C {
    private x: int;
  }

  inner class B extends C {
    public getX(): int {
      return this.x;
    }
  }
}
