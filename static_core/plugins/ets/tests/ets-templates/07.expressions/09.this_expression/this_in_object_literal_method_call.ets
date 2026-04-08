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
desc: This in object literal method can access other object literal methods.
assert: If a method is declared in an object literal, then this is the object literal type.
---*/

interface aClass {
  x: int;
  method(): int;
  helper(): int;
}

class AnotherClass {
  anotherMethod(): aClass {
    const obj: aClass = {
      x: 11,
      helper (): int {
        return this.x;
      },
      method (): int {
        return this.helper();
      }
    };
    return obj;
  }
}

function main(): void {
  let another: AnotherClass = new AnotherClass();
  let obj: aClass = another.anotherMethod();
  arktest.assertEQ(obj.method(), 11);
}
