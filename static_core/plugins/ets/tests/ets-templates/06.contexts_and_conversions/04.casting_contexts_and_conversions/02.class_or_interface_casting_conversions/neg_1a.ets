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
desc: >-
    A narrowing reference casting conversion converts an expression of
    a supertype (superclass or superinterface) to a subclass or subinterface.
tags: [compile-only, negative]
---*/
class Base {}
class Derived1 extends Base {}
class Derived2 extends Base {}

function main() {
    let b: Base[] = new Derived1[1]
    let d = b as Derived2[]
}
