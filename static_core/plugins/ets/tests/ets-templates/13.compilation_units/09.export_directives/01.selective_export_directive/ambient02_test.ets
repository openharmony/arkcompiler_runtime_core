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
files: ['./ambient02/file.ets']
---*/

import { foo1, A1, bar1 } from './ambient02/file.d.ets'

function main() {
    assertEQ(foo1(), 142)
    assertEQ(bar1, 42)
    assertEQ(new A1().fld, "abc")
}
