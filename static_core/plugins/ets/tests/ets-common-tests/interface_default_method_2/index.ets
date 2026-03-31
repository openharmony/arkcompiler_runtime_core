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
  files: [declare.ets]
---*/

import { I, I2, C } from './declare.d.ets'

interface I3 extends I {
    
}

class C2 implements I {

}

let i1: I = {};
let i2: I2 = {};
let i3: I3 = {};
let c1: C = {}
let c2: C2 = {}

arktest.assertEQ(i1.foo('Ifoo'), 'Ifoo');
arktest.assertEQ(i2.foo('Ifoo'), 'Ifoo');
arktest.assertEQ(i3.foo('Ifoo'), 'Ifoo');
arktest.assertEQ(c1.foo('Ifoo'), 'Ifoo');
arktest.assertEQ(c2.foo('Ifoo'), 'Ifoo');
