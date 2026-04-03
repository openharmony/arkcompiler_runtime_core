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
desc: Test for binary compatibility. Reordering of members such as fields, methods.
files: ['./reorder_members.libr.ets', '../../param_interfaces/param_interfaces.ets']
---*/

import * as PI from '../../param_interfaces/param_interfaces'
import * as LIB from './reorder_members.libr.ets'

function main(): void {
    let c1 = new LIB.C1;
    let c2 = new LIB.C2;
    let c3 = new LIB.C3;

    arktest.assertEQ(c1.m(c3.pi3), "m(PI3) from C1", "m(PI3) from C1 call expected");
    arktest.assertEQ(c1.m(c3.pi4), "m(PI4) from C1", "m(PI4) from C1 call expected");
    arktest.assertEQ(c1.m(c3.pi6), "m(PI6) from I1", "m(PI6) from I1 call expected");

    arktest.assertEQ(c2.foo(1), "foo(number) from C2", "foo(number) from C2 call expected");
    arktest.assertEQ(c2.foo(false), "foo(boolean) from C2", "foo(boolean) from C2 call expected");

    arktest.assertEQ(c3.foo1(), "foo1() from C3", "foo1() from C3 call expected");
    arktest.assertEQ(c3.foo2(), "foo2() from C3", "foo2() from C3 call expected");
}
