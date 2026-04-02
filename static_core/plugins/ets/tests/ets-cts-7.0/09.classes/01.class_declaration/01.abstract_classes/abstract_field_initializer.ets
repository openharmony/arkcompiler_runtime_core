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
desc: >-
    Field initializers for non-static fields of an abstract class are executed
    when a non-abstract subclass is instantiated.
---*/

abstract class Base {
  field: number = 10 + 5
}

class Derived extends Base {
  constructor() {}
}

function main(): void {
  let d: Derived = new Derived();
  arktest.assertEQ(d.field, 15);
}
