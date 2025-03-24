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
desc: Top-level declarations can be made exported by using a selective export directive.
files: ['./ambient03/file.ets']
---*/

import { ns1 as ns } from './ambient03/file.d.ets'

function main() {
    assertEQ(ns.foo(), 142)
    assertEQ(ns.bar, 42)
    assertEQ(new ns.A().fld, "abc")
}
