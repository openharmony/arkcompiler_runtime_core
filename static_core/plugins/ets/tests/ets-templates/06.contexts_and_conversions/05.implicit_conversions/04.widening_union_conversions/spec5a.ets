/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: Conversion from a union type to a wider union type.
---*/

class Base {}
class Derived1 extends Base {}
class Derived2 extends Base {}

function main() {
    let u4: Base | Derived1 | Derived2 = new Derived2()
    let u5: Derived1 | Derived2 = new Derived2()
    u4 = u5 // OK, u4 type is Base after normalization and Derived1 and Derived2
            // are compatible with Base as Note states
}
