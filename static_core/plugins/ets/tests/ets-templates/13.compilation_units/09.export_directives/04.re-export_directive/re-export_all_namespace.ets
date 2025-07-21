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
desc: Re-export directive
files: ['./module_b_re-export.ets', './module_b.ets']
---*/

import { name } from "./module_b_re-export"

function main() : void
{
    // Export function
    assertEQ(name.bar(), 100);

    // Export class
    let foo_cls : name.foo = new name.foo();
    assertEQ(foo_cls.foo_bar(), 101);
    assertEQ(name.foo.foo_bar_s(), 102);

    // Export interface
    let foo_inter : name.foo_Inter = {fld1: 10, fld2: "Hi"};
    assertEQ(foo_inter.fld1, 10);
    assertEQ(foo_inter.fld2, "Hi");

    // Export enum
    assertEQ(name.Color.Red.valueOf(), 10);
    assertEQ(name.Color.Blue.valueOf(), 20);

    // Export namespace
    assertEQ(name.NS.Color.Red.valueOf(), 5);
    assertEQ(name.NS.Color.Blue.valueOf(), 10);

    // Export variable
    assertEQ(name.numExported, 111);

    // Export annotation
    @name.MyAnno let anno : int = 10;

    // Export type alias
    let foo_cls1 : name.fooCls = new name.foo();
    assertEQ(foo_cls1.foo_bar(), 101);
}