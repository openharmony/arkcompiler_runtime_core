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
desc: Type of this in interface method.
assert: A corresponding interface type is the type of this expression when used in interface method.
---*/

interface anInterface {
  x: int;

  method(): anInterface {
    return this; // type of 'this' is anInterface
  }
}

class ImplClass implements anInterface {
  public x: int = 33;

  method(): anInterface {
    arktest.assertTrue(this instanceof anInterface);
    return this;
  }
}

function main(): void {
  let instance: ImplClass = new ImplClass();
  let result: anInterface = instance.method();

  arktest.assertEQ(result.x, 33);
}
