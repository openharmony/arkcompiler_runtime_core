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
desc: null-coalescing operator with other lower precedence operators
---*/

interface I {}
class A implements I {}
class B {}

function main(): int {
    let a: A = new A();
    let b: B = new B();
    let i: I = a;
    let an: A|null = null;
    let bn: B|null = null;

    // ternary
    let res: boolean = ((an ?? a) instanceof I ? bn ?? b : an ?? a) === b
    if (!res) return 1;

    res = ((a ?? an) instanceof B ? an ?? a : bn ?? b) === b
    if (!res) return 1;

    // assignment
    let x: Object = bn ?? a
    if (x !== a) return 1;

    let y: Object = an ?? b
    if (y !== b) return 1;

    return 0;
}
