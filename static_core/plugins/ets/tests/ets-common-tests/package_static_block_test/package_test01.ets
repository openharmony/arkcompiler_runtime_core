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
  files: [./package_with_static_block/package01.ets]
---*/

import {nn, NS, ME, bb, cc, ss} from "./package_with_static_block/package01"
import {in_bb, in_cc, in_ss, in_nn} from "./package_with_static_block/package01"
import {c_nn, c_bb, c_cc, c_ss} from "./package_with_static_block/package01"
import {c_in_bb, c_in_cc, c_in_ss, c_in_nn} from "./package_with_static_block/package01"
assertEQ(nn, 6)
assertEQ(NS.n, 66)
assertEQ(NS.NS1.n_sub, 666)
assertEQ(bb, true)
assertEQ(cc, c'c')
assertEQ(ss, "hello")

assertEQ(in_bb, true)
assertEQ(in_cc, c'c')
assertEQ(in_ss, "world")
assertEQ(in_nn, 666)

assertEQ(c_nn, 1)
assertEQ(c_bb, true)
assertEQ(c_cc, c'c')
assertEQ(c_ss, "hello")

assertEQ(c_in_bb, true)
assertEQ(c_in_cc, c'c')
assertEQ(c_in_ss, "world")
assertEQ(c_in_nn, 666)
