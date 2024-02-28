/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
    Shadowing is the compile-time error that occurs if an imported function is identical to the function declared in the
    current compilation unit (the same names and override-equivalent signatures), i.e., the declarations are duplicated.
tags: [compile-only, negative]
---*/

import {max} from "std/math";

function max(v: double, u: double): double {
    assert false
    return v > u ? v : u;
}

function main(): void {
    max(1.0, 1.0)
}
