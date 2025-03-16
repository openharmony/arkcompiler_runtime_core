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
desc: A class inherits from its direct superinterface.
assert: A class inherits from its direct superclass and direct superinterfaces all non-private fields of the superclass and superinterfaces that are both accessible to code in the class and not hidden by a declaration in the class.
---*/

interface A {
  x: int;
}

interface C {
  y: boolean;
}

class B implements A, C{
  x: int;
  y: boolean;
}

function main(): void {
    let instance: B = new B();

    assertEQ( instance.x, 0 );
    assertEQ( instance.y, false );
}
