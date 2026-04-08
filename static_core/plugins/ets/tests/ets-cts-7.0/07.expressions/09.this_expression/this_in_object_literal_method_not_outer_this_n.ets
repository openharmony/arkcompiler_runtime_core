/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: This in object literal method should not be captured from outer class.
assert: >-
    If a method is declared in an object literal, then the type of this
    expression is the type of the object literal.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0087: Property 'y' does not exist on type 'aClass'"
---*/

interface aClass {
  x: int;
  method(): int;
  field: () => int;
}

class AnotherClass {
  public y: int = 100;

  anotherMethod(): aClass {
    const obj: aClass = {
      x: 11,
      method (): int {
        // Must fail: this is aClass, not AnotherClass.
        return this.y;
      },
      field: (): int => {
        return this.y;
      }
    };
    return obj;
  }
}

function main(): void {
}
