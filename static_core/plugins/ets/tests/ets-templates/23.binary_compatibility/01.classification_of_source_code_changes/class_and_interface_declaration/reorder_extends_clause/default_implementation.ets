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
desc: Test for binary compatibility. Reordered types in extends clause.
files: ['./default_implementation.libr.ets', '../../param_interfaces/param_interfaces.ets']
---*/

import * as PI from '../../param_interfaces/param_interfaces'
import * as LIB from './default_implementation.libr.ets'

class C1 implements LIB.I3 {}

function main(): void {
    let c1 = new C1;
    let pi1: PI.PI1 = {};
    arktest.assertEQ(c1.m(pi1), "I1", "m() from I1 call expected");
}
