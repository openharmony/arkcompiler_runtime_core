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

import { I1, I2 } from './declare.d.ets'

let i11: I1 = {
  foo1(s: string) {
    return s;
  }
};

let i12: I1 = {
  foo1: (s: string) => {
    return s;
  }
};

let i21: I2 = {
  foo2(s: int){
    return s;
  }
};

let i22: I2 = {
  foo2: (s: int) => {
    return s;
  }
};

arktest.assertEQ(i11.foo('Ifoo'), 'Ifoo');
arktest.assertEQ(i11.foo1('Ifoo11'), 'Ifoo11');
arktest.assertEQ(i12.foo('Ifoo'), 'Ifoo');
arktest.assertEQ(i12.foo1('Ifoo12'), 'Ifoo12');
arktest.assertEQ(i21.foo('Ifoo21'), 'Ifoo21');
arktest.assertEQ(i21.foo2(21), 21);
arktest.assertEQ(i22.foo('Ifoo22'), 'Ifoo22');
arktest.assertEQ(i22.foo2(22), 22);
