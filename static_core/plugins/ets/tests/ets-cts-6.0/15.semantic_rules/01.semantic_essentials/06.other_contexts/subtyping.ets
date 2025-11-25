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
desc: The only semantic rule for all other contexts, and specifically for Overriding, is to use Subtyping.
---*/

class Base {}

class Derived extends Base {}

function want_base(x: Base) {}

function main() {
    let a = new Base()
    let b: Base = new Derived()
    let c: Derived = new Derived()
    want_base(a)
    want_base(b)
    want_base(c)
}
