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
desc: Test for binary compatibility. Inserting class with no member to the type hierarchy.
files: ['./insert_class.libr.ets', '../../param_interfaces/param_interfaces.ets']
---*/

import * as PI from '../../param_interfaces/param_interfaces'
import * as LIB from './insert_class.libr.ets'

function main(): void {
    let c3 = new LIB.C3;
    let pi1: PI.PI1 = {};
    let pi3: PI.PI3 = {};
    let pi4: PI.PI4 = {};
    let pi5: PI.PI5 = {};
    arktest.assertEQ(c3.m(pi1), "I5", "m() from I5 call expected");
    arktest.assertEQ(c3.m(pi3), "C1", "m() from C1 call expected");
    arktest.assertEQ(c3.m(pi4), "I3", "m() from I3 call expected");
    arktest.assertEQ(c3.m(pi5), "C1", "m() from C1 call expected");
}
