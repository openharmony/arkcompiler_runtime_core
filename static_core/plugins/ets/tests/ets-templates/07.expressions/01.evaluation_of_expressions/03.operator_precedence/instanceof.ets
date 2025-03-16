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
desc: instanceof operator with other lower precedence operators
---*/

interface I {}
class A implements I {}
class B {}

function main(): int {
    let a: A = new A();
    let b: B = new B();
    let i: I = a;
    let an: A|null = null;

    // equality
    let res: boolean = a instanceof A == i instanceof A
    if (!res) return 1;

    res = a instanceof A != b instanceof A
    if (!res) return 1

    // ternary
    res = i instanceof A ? b instanceof A : b instanceof B
    if (res) return 1;

    res = i instanceof B ? b instanceof A : b instanceof B
    if (!res) return 1;

    return 0;
}
