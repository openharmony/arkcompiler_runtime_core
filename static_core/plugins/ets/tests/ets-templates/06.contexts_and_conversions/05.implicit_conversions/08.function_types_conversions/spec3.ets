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
desc: Function types conversion is the conversion of one function type to another.
---*/

class Base {}
class Derived extends Base {}

function main() {
    let foo1: (p: Base) => Base = (p: Base): Derived => new Derived()
    /* OK: identical (invariant) parameter types, and compatible return type */

    let foo2: (p: Derived) => Base = (p: Base): Derived => new Derived()
    /* OK: contravariant parameter types, and compatible return type */
}
