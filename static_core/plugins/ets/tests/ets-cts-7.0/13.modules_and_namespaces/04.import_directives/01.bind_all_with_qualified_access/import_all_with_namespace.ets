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
desc: star import allows accessing exported let, function, class, and nested namespace members
files:
- ./modules/mod_with_ns.ets
tags: [no-warmup]
---*/

import * as ns from "./modules/mod_with_ns"

function main() {
    arktest.assertEQ(ns.exportedLet, null)
    ns.exportedLet = 42
    arktest.assertEQ(ns.exportedLet, 42)
    arktest.assertEQ(ns.exportedFunc(), 77)
    arktest.assertEQ(new ns.ExportedClass().getValue(), "clsval")
    arktest.assertEQ(ns.ExportedNS.nsVar, 55)
    ns.ExportedNS.nsVar = 99
    arktest.assertEQ(ns.ExportedNS.nsVar, 99)
    arktest.assertEQ(ns.ExportedNS.nsFunc(), true)
}
